use std::fs::OpenOptions;
use std::io::BufWriter;
use std::num::NonZeroUsize;
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::{mem, ptr};
use std::sync::{RwLock, Arc, Mutex};

use serde::Serialize;

use rand::Rng;

use crate::random_tries::{chop_ys, SignalStrength, BoundingBoxes};
use crate::room_state::{Pos, Px, RadioPoint, RadioZone, RoomLayout, Wall};

/// 5 GHz
const CARRYING_FREQ: f64 = 5f64;

//pub const STATIC_NOISE_DBM:f64 = 0.2;
pub const STATIC_NOISE_DBM:f64 = -110.0;

// Для закраски используем таблицу переходов между цветами.
//  хранит точку, где цвет наиболее интенсивный.
// r, g, b хранят цвет п палитре RGB.
struct Segment {
  t: f64,
  r: u8,
  g: u8,
  b: u8,
}

struct State {
  scene: Box<[u8]>,
}

pub fn length(a: Pos, b: Pos) -> f64 {
  length_sq(a, b).sqrt()
}

pub fn length_sq(a: Pos, b: Pos) -> f64 {
  let dx = a.x - b.x;
  let dy = a.y - b.y;
  dx*dx + dy*dy
}

pub fn dot(a: Pos, b: Pos) -> f64 {
  a.x*b.x + a.y*b.y
}

pub fn bounding_box(pos: impl Iterator<Item = impl Deref<Target = Pos>>) -> (Pos, Pos) {
  let mut min = Pos::new(f64::MAX, f64::MAX);
  let mut max = Pos::new(f64::MIN, f64::MIN);
  for p in pos {
    let p: &Pos = p.deref();
    min.x = p.x.min(min.x);
    max.x = p.x.max(max.x);
    min.y = p.y.min(min.y);
    max.y = p.y.max(max.y);
  }
  return (min, max)
}

pub fn is_inside(point: Pos, zone: &RadioZone) -> bool {
  let (min, max) = bounding_box(zone.points.iter());
  if point.x < min.x || point.x > max.x || point.y < min.y || point.y > max.y {
    return false;
  }

  let mut inside = false;
  let mut i = 0;
  let mut j = zone.points.len() - 1;
  while i != zone.points.len() {
    let (a, b) = unsafe {(
      *zone.points.get_unchecked(i),
      *zone.points.get_unchecked(j)
    )};

    inside ^= (point.y < a.y) != (point.y < b.y) && (point.x - a.x) < (point.y - a.y) * (b.x-a.x) / (b.y-a.y);

    i += 1;
    j = i - 1;
  }
  return inside;
}

fn distance_to_zone(pix: Pos, zone: &RadioZone) -> f64 {
  if is_inside(pix, zone) {
    return 0.0;
  }

  let mut min_len = f64::MAX;
  let mut i = 0;
  let mut j = zone.points.len() - 1;
  while i != zone.points.len() {
    let (v, w) = unsafe {(
      *zone.points.get_unchecked(i),
      *zone.points.get_unchecked(j)
    )};

    let d = w - v;
    let len_sq = d.x*d.x + d.y*d.y;
    let t = (dot(pix - v, w - v) / len_sq).clamp(0.0, 1.0);
    let projection = v + d * t;
    let len = length(pix, projection);
    if len < min_len {
      min_len = len;
    }

    i += 1;
    j = i - 1;
  }
  return min_len;
}

fn scale_dbs2(v: f64, min: f64, max: f64, pow: f64) -> f64 {
  ((v - min) / (max - min)).clamp(0.0, 1.0).powf(pow)
}

pub fn dbm_to_mw(dbm: f64) -> f64 {
  10f64.powf(dbm / 10f64)
}

pub fn mw_to_dbm(mw: f64) -> f64 {
  10f64 * mw.log10()
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn do_parallel<'a, T>(
  bb: Arc<BoundingBoxes>,
  starting_t: &mut T,
  map: Arc<impl Fn(Pos, &mut T) -> () + Send + Sync + 'static>,
  reduce: impl Fn(&mut T, T) -> () + Send + Sync + 'static,
) where
T: Clone + Send + Sync + 'static,
{
  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let mut queued = Vec::with_capacity(threads);

  for (y_from, y_to, _) in chop_ys(bb.res.1, threads) {
    let map = map.clone();
    let bb = bb.clone();

    let mut merged = starting_t.clone();

    queued.push(tauri::async_runtime::spawn(async move {
      for y in y_from..y_to {
        for x in 0..bb.res.0 {
          let pixel = pix_to_meter(&bb, Px::new(x as isize, y as isize));

          let _: () = (map)(pixel, &mut merged);
        }
      }

      return merged;
    }));
  }
  for queued in queued {
    let _: () = (reduce)(starting_t, queued.await.unwrap());
  }
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn do_image<'a, F>(
  bb: Arc<BoundingBoxes>,
  png_path: &'static str,
  func: Arc<F>,
) where
F: Fn(Pos) -> (u8, u8, u8) + Send + Sync + 'static,
{
  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let state_arc = Arc::new(RwLock::new(vec![0u8; bb.res.0*bb.res.1*3].into_boxed_slice()));

  let mut queued = Vec::with_capacity(threads);

  for (y_from, y_to, _) in chop_ys(bb.res.1, threads) {
    let state = state_arc.clone();
    let func = func.clone();
    let bb = bb.clone();

    queued.push(tauri::async_runtime::spawn(async move {
      let state = state.read().unwrap();
      // Перебираем все пиксели на холсте
      for y in y_from..y_to {
        for x in 0..bb.res.0 {
          let pixel = pix_to_meter(&bb, Px::new(x as isize, y as isize));

          // Рассматриваем каждую точку
          let (r, g, b) = (func)(pixel);

          // Закрашиваем пиксель
          unsafe {
            let offset = state.as_ptr().add((y * bb.res.0 + x) * 3) as *mut u8;
            ptr::write(offset.add(0), r);
            ptr::write(offset.add(1), g);
            ptr::write(offset.add(2), b);
          }
        }
      }
    }));
  }
  for queued in queued {
    queued.await.unwrap();
  }

  let mut state = state_arc.write().unwrap();
  let state = state.deref_mut().deref_mut();
  save_image(bb.res, state, png_path);
}

#[derive(Clone, Serialize)]
pub struct ActiveBestZone {
  point_x: f64,
  point_y: f64,
  point_pow_mw: f64,
  min_sinr_dbm: f64,
  min_sinr_x: f64,
  min_sinr_y: f64,
  r: u8,
  g: u8,
  b: u8,
}

struct ActiveBest {
  zones: Vec<ActiveBestZone>,
}

static ACTIVE_BEST: Mutex<ActiveBest> = Mutex::new(ActiveBest { zones: Vec::new() });

#[tauri::command]
pub fn get_active_best() -> Vec<ActiveBestZone> {
  ACTIVE_BEST.lock().unwrap().zones.clone()
}

fn sum_dbm(dbm1: f64, dbm2: f64) -> f64 {
  if dbm1 < dbm2 {
    return dbm1 + 10.0 * f64::log10(1.0 + f64::powf(10.0, (dbm2 - dbm1) / 10.0));
  } else {
    return dbm2 + 10.0 * f64::log10(1.0 + f64::powf(10.0, (dbm1 - dbm2) / 10.0));
  }
}

pub fn do_calc_sinr_dbm(
  room_layout: &RoomLayout,
  bsp: &BSP,
  next_guess: &SignalStrength,
  zone: &RadioZone,
  pix: Pos,
) -> f64 {
  let dbms = calc_powers_dbm(room_layout, &bsp, next_guess, pix);
  return powers_to_sinr(&dbms, zone);
}

fn powers_to_sinr(dbms: &[f64], zone: &RadioZone) -> f64 {/*
  // let signal_mw = mwts[zone.desired_point_id];
  // let all_signals_mw = mwts.iter().sum::<f64>();
  // let interference_mw = all_signals_mw - signal_mw;

  let mut signal_dbm = -200.0;
  let mut interference_dbm = -200.0;

  for (i, dbm) in dbms.iter().enumerate() {
    if i == zone.desired_point_id {
      signal_dbm = sum_dbm(signal_dbm, *dbm);
    } else {
      interference_dbm = sum_dbm(interference_dbm, *dbm);
    }
  }

  // signal - all signals = (s)/(s+i+n)
  // max limit: 400mw на точке доступа
  // min limit: -5db в худшей точке зоны.
  let int_noise_dbm = sum_dbm(interference_dbm, STATIC_NOISE_DBM);
  // let int_noise_mw = interference_mw + dbm_to_mw(STATIC_NOISE_DBM);
  let sinr = mw_to_dbm(dbm_to_mw(signal_dbm) / dbm_to_mw(int_noise_dbm));
  // let sinr = mw_to_dbm(signal_mw / int_noise);
*/
  // println!("signals: {:?}dbms", &dbms);
  // println!("sig: {}dbm", signal_dbm);
  // println!("int: {}dbm", interference_dbm);
  // println!("int+noise: {}dbm", int_noise_dbm);
  // println!("sinr: {}", sinr);
  // println!("");

  /////////////////

  // let signal_mw = mwts[zone.desired_point_id];
  // let all_signals_mw = mwts.iter().sum::<f64>();
  // let interference_mw = all_signals_mw - signal_mw;

  let mut signal_mw = 0.0;
  let mut interference_mw = 0.0;

  for (i, dbm) in dbms.iter().enumerate() {
    if i == zone.desired_point_id {
      signal_mw += dbm_to_mw(*dbm);
    } else {
      interference_mw += dbm_to_mw(*dbm);
    }
  }

  // signal - all signals = (s)/(s+i+n)
  // max limit: 400mw на точке доступа
  // min limit: -5db в худшей точке зоны.
  let int_noise_mw = interference_mw + dbm_to_mw(STATIC_NOISE_DBM);
  // let sinr = mw_to_dbm(signal_mw / int_noise_mw);
  let sinr = signal_mw / int_noise_mw;


  // let signal = mws[zone.desired_point_id];
  // let all_signals = mws.iter().sum::<f64>();
  // let interference = all_signals - signal;
  // signal - all signals = (s)/(s+i+n)
  // max limit: 400mw на точке доступа
  // min limit: -5db в худшей точке зоны.
  // let sinr = mw_to_dbm(signal / (interference + dbm_to_mw(STATIC_NOISE_DBM)));

  return sinr;
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn next_image(
  bb: Arc<BoundingBoxes>,
  room_layout: Arc<RoomLayout>,
  bsp: Arc<BSP>,
  next_guess: Arc<SignalStrength>,
) {
  let mut min_max_sinr_per_zone = vec![(f64::MAX, f64::MIN, f64::MAX, Pos { x: 0.0, y: 0.0 }); room_layout.radio_zones.len()].into_boxed_slice();
  {
    let next_guess = next_guess.clone();
    let room_layout = room_layout.clone();
    let bsp = bsp.clone();
    do_parallel::<Box<[_]>>(bb.clone(), &mut min_max_sinr_per_zone, Arc::new(move |pix, res: &mut Box<[(f64, f64, f64, Pos)]>| {
      for ((min, max, min_pwr, min_xy), zone) in res.iter_mut().zip(room_layout.radio_zones.iter()) {
        if is_inside(pix, zone) {
          let dbms = calc_powers_dbm(&room_layout, &bsp, &next_guess, pix);
          let sinr = powers_to_sinr(&dbms, zone);
          let power_dbm = dbms[zone.desired_point_id];
          if power_dbm < *min_pwr {
            *min_pwr = power_dbm;
          }
          if sinr < *min {
            *min = sinr;
            *min_xy = pix;
          }
          if sinr > *max {
            *max = sinr;
          }
        }
      }
    }), move |res, new| {
      for ((g_min, g_max, g_min_pwr, g_pos), (l_min, l_max, l_min_pwr, l_pos)) in res.iter_mut().zip(new.into_iter()) {
        *g_max = g_max.max(*l_max);
        *g_min_pwr = g_min_pwr.min(*l_min_pwr);
        if l_min < g_min {
          *g_min = *l_min;
          *g_pos = *l_pos;
        }
      }
    }).await;
  }

  let mut active_best = Vec::new();

  for ((((min, max, min_pwr, pos), zone), pow_mw), point_pos) in min_max_sinr_per_zone.iter()
    .zip(room_layout.radio_zones.iter())
    .zip(next_guess.points_signal_mws.iter())
    .zip(room_layout.radio_points.iter())
  {
    active_best.push(ActiveBestZone {
      point_x: point_pos.pos.x,
      point_y: point_pos.pos.y,
      point_pow_mw: *pow_mw,
      min_sinr_dbm: *min,
      min_sinr_x: pos.x,
      min_sinr_y: pos.y,
      r: zone.r,
      g: zone.g,
      b: zone.b,
    });
    println!("zone: r{} g{} b{} => SINR Min: {}, SINR Max: {}, min_pwr: {}, powers_mws: {:?}", zone.r, zone.g, zone.b, min, max, min_pwr, next_guess.points_signal_mws.as_slice())
  }

  let min_max_sinr_per_zone = Arc::<[_]>::from(min_max_sinr_per_zone);

  {
    let next_guess = next_guess.clone();
    let room_layout = room_layout.clone();
    let min_max_sinr_per_zone = min_max_sinr_per_zone.clone();
    let bsp = bsp.clone();
    do_image(bb.clone(), "../rimg3.png", Arc::new(move |pix| {
      let mut interp_a = [ 0.0, 0.0, 0.0 ];
      let mut interp_b = [ 0.0, 0.0, 0.0 ];
      let iter = room_layout.radio_zones.iter()
        .zip(min_max_sinr_per_zone.iter());
      for (zone, (min_sinr, max_sinr, _min_pwr, _min_pos)) in iter {
        let zone_rgb = [zone.r, zone.g, zone.b];
        // let d = distance_to_zone(pix, zone).powf(2.0).max(0.00001);
        let d = distance_to_zone(pix, zone).max(0.00001);

        let sinr = do_calc_sinr_dbm(&room_layout, &bsp, &next_guess, zone, pix);
        let sinr_01 = scale_dbs2(sinr, *min_sinr, *max_sinr, 1.0);
        let sinr_01 = (sinr_01 * 12.0).round() / 12.0;

        for i in 0..3 {
          let color = (255.0 * (1.0 - sinr_01)) + (zone_rgb[i] as f64 * sinr_01);
          interp_a[i] += color / d;
          interp_b[i] += 1.0 / d;
        }
      }

      let r = (interp_a[0] / interp_b[0]).clamp(0.0, 255.0) as u8;
      let g = (interp_a[1] / interp_b[1]).clamp(0.0, 255.0) as u8;
      let b = (interp_a[2] / interp_b[2]).clamp(0.0, 255.0) as u8;

      return (r, g, b);
    })).await;
  }

  {
    let next_guess = next_guess.clone();
    let room_layout = room_layout.clone();
    let min_max_sinr_per_zone = min_max_sinr_per_zone.clone();
    let bsp = bsp.clone();
    let bb = bb.clone();
    do_image(bb, "../rimg1.png", Arc::new(move |pix| {
      let mut interp_a = [ 0.0, 0.0, 0.0 ];
      let mut interp_b = [ 0.0, 0.0, 0.0 ];
      let iter = room_layout.radio_zones.iter()
        .zip(min_max_sinr_per_zone.iter());
      for (zone, (min_sinr, max_sinr, _min_pwr, _min_pos)) in iter {
        // let d = distance_to_zone(pix, zone).powf(2.0).max(0.00001);
        let d = distance_to_zone(pix, zone).max(0.00001);

        let sinr = do_calc_sinr_dbm(&room_layout, &bsp, &next_guess, zone, pix);
        let s = scale_dbs2(sinr, *min_sinr, *max_sinr, 1.0);

        // Делаем плавные переходы ступенчатыми (40 ступеней)
        let s = (s * 40.0).round() / 40.0;
        // Градиент переходов между цветами
        const GRAD: [Segment; 8] = [
          Segment { t: -1.000, r: 0xFF, g: 0xFF, b: 0xFF, },
          Segment { t: -0.000, r: 0xFF, g: 0xFF, b: 0xFF, },
          Segment { t:  0.200, r: 0x00, g: 0x00, b: 0xFF, },
          Segment { t:  0.400, r: 0x00, g: 0xCF, b: 0xFF, },
          Segment { t:  0.600, r: 0x00, g: 0xCF, b: 0x00, },
          Segment { t:  0.800, r: 0xFF, g: 0xCF, b: 0x00, },
          Segment { t:  1.000, r: 0xFF, g: 0x00, b: 0x00, },
          Segment { t:  2.000, r: 0xFF, g: 0x00, b: 0x00, },
        ];
        let mut rgb = [0.0;3];
        for i in 0..(GRAD.len()-1) {
          let g0 = &GRAD[i+0];
          let g1 = &GRAD[i+1];
          // Проверяем, попадаем ли мы в нужный диапазон
          if s >= g0.t && s <= g1.t {
            let t = (s - g0.t) / (g1.t - g0.t);
            rgb[0] = g0.r as f64 * (1.0 - t) + g1.r as f64 * t;
            rgb[1] = g0.g as f64 * (1.0 - t) + g1.g as f64 * t;
            rgb[2] = g0.b as f64 * (1.0 - t) + g1.b as f64 * t;
            break;
          }
        }
        for i in 0..3 {
          interp_a[i] += rgb[i] as f64 / d;
          interp_b[i] += 1.0 / d;
        }
      }

      let r = (interp_a[0] / interp_b[0]).clamp(0.0, 255.0) as u8;
      let g = (interp_a[1] / interp_b[1]).clamp(0.0, 255.0) as u8;
      let b = (interp_a[2] / interp_b[2]).clamp(0.0, 255.0) as u8;

      return (r, g, b);
    })).await;
  }

  {
    let next_guess = next_guess.clone();
    let room_layout = room_layout.clone();
    let min_max_sinr_per_zone = min_max_sinr_per_zone.clone();
    let bsp = bsp.clone();
    let bb = bb.clone();
    do_image(bb, "../rimg2.png", Arc::new(move |pix| {
      let mut interp_a = [ 0.0, 0.0, 0.0 ];
      let mut interp_b = [ 0.0, 0.0, 0.0 ];
      let mut dbms = calc_powers_dbm(&room_layout, &bsp, &next_guess, pix);
      let iter = dbms.iter_mut()
        .zip(min_max_sinr_per_zone.iter())
        .zip(next_guess.points_signal_mws.iter());
      for ((dbm, (_min_sinr, _max_sinr, min_pwr, _min_pos)), power_mw) in iter {
        *dbm = scale_dbs2(*dbm, *min_pwr, mw_to_dbm(*power_mw), 1.0/4.0);
      }
      let iter = room_layout.radio_zones.iter()
        .zip(dbms.iter());
      for (zone, s) in iter {
        // let d = distance_to_zone(pix, zone).powf(2.0).max(0.00001);
        let d = distance_to_zone(pix, zone).max(0.00001);

        // Делаем плавные переходы ступенчатыми (40 ступеней)
        // let s = (s * 40.0).round() / 40.0;
        let s = *s;
        // Градиент переходов между цветами
        const GRAD: [Segment; 8] = [
          Segment { t: -1.000, r: 0xFF, g: 0x00, b: 0x00, },
          Segment { t: -0.000, r: 0xFF, g: 0x00, b: 0x00, },
          Segment { t:  0.200, r: 0xFF, g: 0xCF, b: 0x00, },
          Segment { t:  0.400, r: 0x00, g: 0xCF, b: 0x00, },
          Segment { t:  0.600, r: 0x00, g: 0xCF, b: 0xFF, },
          Segment { t:  0.800, r: 0x00, g: 0x00, b: 0xFF, },
          Segment { t:  1.000, r: 0xFF, g: 0xFF, b: 0xFF, },
          Segment { t:  2.000, r: 0xFF, g: 0xFF, b: 0xFF, },
        ];
        let mut rgb = [0.0;3];
        for i in 0..(GRAD.len()-1) {
          let g0 = &GRAD[i+0];
          let g1 = &GRAD[i+1];
          // Проверяем, попадаем ли мы в нужный диапазон
          if s >= g0.t && s <= g1.t {
            let t = (s - g0.t) / (g1.t - g0.t);
            rgb[0] = g0.r as f64 * (1.0 - t) + g1.r as f64 * t;
            rgb[1] = g0.g as f64 * (1.0 - t) + g1.g as f64 * t;
            rgb[2] = g0.b as f64 * (1.0 - t) + g1.b as f64 * t;
            break;
          }
        }
        for i in 0..3 {
          interp_a[i] += rgb[i] as f64 / d;
          interp_b[i] += 1.0 / d;
        }
      }

      let r = (interp_a[0] / interp_b[0]).clamp(0.0, 255.0) as u8;
      let g = (interp_a[1] / interp_b[1]).clamp(0.0, 255.0) as u8;
      let b = (interp_a[2] / interp_b[2]).clamp(0.0, 255.0) as u8;

      return (r, g, b);
    })).await;
  }

  {
    ACTIVE_BEST.lock().unwrap().zones = active_best;
  }

  println!("drew");
}

pub fn calc_powers_dbm(layout: &RoomLayout, bsp: &BSP, signal_strength: &SignalStrength, pix: Pos) -> Box<[f64]> {
  layout.radio_points
    .iter()
    .zip(signal_strength.points_signal_mws.iter())
    .zip(signal_strength.points_directions_rad.iter())
    .map(|((point, power_mw), point_dir)| dbm_after_walls(layout, bsp, pix, point, *power_mw, *point_dir))
    .collect::<Vec<_>>()
    .into_boxed_slice()
}

pub fn clamp_rad(mut rad: f64) -> f64 {
  while rad < -std::f64::consts::PI {
    rad += 2.0 * std::f64::consts::PI;
  }
  while rad > std::f64::consts::PI {
    rad -= 2.0 * std::f64::consts::PI;
  }
  return rad;
}

// Находит силу сигнала от точки в определённом пикселе.
pub fn dbm_after_walls(room_layout: &RoomLayout, bsp: &BSP, pixel: Pos, point: &RadioPoint, power_mw: f64, antena_dir: f64) -> f64 {
  let power_dbm = mw_to_dbm(power_mw);

  let length = length(point.pos, pixel).max(1.0);

  let dir = pixel - point.pos;
  let angle = f64::atan2(dir.y, dir.x) - antena_dir;
  let a_div = clamp_rad(angle) / 65.0f64.to_radians();
  let dir_gain = -f64::min(30.0, 12.0 * a_div * a_div);

  // Находим где находится точко относительно текущего пикселя
  let path_loss = 38.3 * f64::log10(length) + 24.9 * f64::log10(CARRYING_FREQ) + 17.3;

  // Смотрим сколько стен пересекаются с лучём видимости.

  let mut wall_loss = 0f64;
  // let mut off = point.pos;
  // let mut iw = None;
  // while let Some((p, w)) = bsp.intersect(off, pixel, iw) {
  //   wall_loss += w.damping;
  //   (off, iw) = (p, Some(w));
  // }
  for wall in &room_layout.walls {
    if line_intersection((pixel, point.pos), (wall.a, wall.b)).is_some() {
      wall_loss += wall.damping;
    }
  }

  return power_dbm - path_loss - wall_loss + dir_gain;
}

fn save_image<P: AsRef<Path>>(res: (usize, usize), scene: &mut [u8], path: P) {
  let file = OpenOptions::new().create(true).write(true).truncate(true).open(&path).unwrap();
  let ref mut w = BufWriter::new(file);

  let mut encoder = png::Encoder::new(w, res.0 as u32, res.1 as u32);
  encoder.set_color(png::ColorType::Rgb);
  encoder.set_depth(png::BitDepth::Eight);
  encoder.set_compression(png::Compression::Best);
  encoder.set_source_gamma(png::ScaledFloat::from_scaled(45455));
  // 1.0 / 2.2, scaled by 100000
  let source_chromaticities = png::SourceChromaticities::new(     // Using unscaled instantiation here
      (0.31270, 0.32900),
      (0.64000, 0.33000),
      (0.30000, 0.60000),
      (0.15000, 0.06000)
  );
  encoder.set_source_chromaticities(source_chromaticities);
  let mut writer = encoder.write_header().unwrap();

  writer.write_image_data(scene).unwrap();
  writer.finish().unwrap();
}

pub fn pix_to_meter(bb: &BoundingBoxes, p: Px) -> Pos {
  Pos::new(
    bb.min.x + p.x as f64 * bb.wh.x / bb.res.0 as f64 + (bb.wh.x / (2 * bb.res.0) as f64),
    bb.min.y + p.y as f64 * bb.wh.y / bb.res.1 as f64 + (bb.wh.y / (2 * bb.res.1) as f64),
  )
}

#[inline(always)]
fn put(width: usize, dest: &mut [u8], x: isize, y: isize, r: u8, g: u8, b: u8) {
  let offset = (y as usize * width + x as usize) * 3;
  unsafe {
    *dest.get_unchecked_mut(offset  ) = r;
    *dest.get_unchecked_mut(offset+1) = g;
    *dest.get_unchecked_mut(offset+2) = b;
  }
  // dest[offset  ] = r;
  // dest[offset+1] = g;
  // dest[offset+2] = b;
}

pub fn line_intersection(a: (Pos, Pos), b: (Pos, Pos)) -> Option<Pos> {
  let da = a.1 - a.0;
  let db = b.1 - b.0;
  let ab0 = a.0 - b.0;
  let s = (da.x * ab0.y - da.y * ab0.x) / (da.x * db.y - db.x * da.y);
  let t = (db.x * ab0.y - db.y * ab0.x) / (da.x * db.y - db.x * da.y);

  if s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0 {
    return Some(Pos::new(a.0.x + (t * da.x), a.0.y + (t * da.y)));
  }
  return None;
}

pub struct BSP {
  splitter: Wall,
  front: Option<Box<BSP>>,
  back: Option<Box<BSP>>,
}
impl BSP {
  fn intersect(&self, r0: Pos, r1: Pos, ignore_wall: Option<&Wall>) -> Option<(Pos, &Wall)> {
    let q = r0;
    let p = self.splitter.a;
    let s = r1 - r0;
    let r = self.splitter.b - self.splitter.a;
    // t = (q − p) × s / (r × s)
    // u = (q − p) × r / (r × s)
    let qp = q - p;
    let rs = Pos::cross(&r, &s);
    let rs_zero = rs.abs() < 0.00001;
    let qps = Pos::cross(&qp, &s);
    let qpr = Pos::cross(&qp, &r);

    // const ray_dir = sub_vec(r1, r0)
    // const segment_dir = sub_vec(self.splitter.b, self.splitter.a)
    // const numerator = cross_prod(sub_vec(self.splitter.a, r0), ray_dir)
    // const denominator = cross_prod(ray_dir, segment_dir)

    // const numerator_is_zero = Math.abs(numerator) < 0.00001

    // if (numerator < 0 || (numerator_is_zero && denominator > 0)) {
    let (near, far) = match qpr < 0.0 || (qpr.abs() < 0.00001 && rs > 0.0) {
      true  => (self.front.as_ref(), self.back.as_ref()),
      false => (self.back.as_ref(), self.front.as_ref()),
    };
    if let Some(near) = near.as_ref() {
      let hit = near.intersect(r0, r1, ignore_wall);
      if hit.is_some() {
        return hit;
      }
    }

    // if the denominator is zero the lines are parallel
    // if (Math.abs(denominator) < 0.00001) {
    if rs_zero {
      return None;
    }

    let t = qps / rs;
    let u = qpr / rs;

    // intersection is the point on a line segment where the line divides it
    // const intersection = numerator / denominator

    // segments that are not parallel and t is in (0, 1) should be divided
    // if (0.0 < intersection && intersection < 1.0) {
    //   return add_vec(self.splitter.a, mul_vec(segment_dir, intersection))
    // }
    if 0.0 < u && u < 1.0 && 0.0 < t && t < 1.0 && !ptr::eq(&self.splitter, ignore_wall.map(ptr::from_ref).unwrap_or(ptr::null())) {
      return Some((q + (s * u), &self.splitter));
    }

    if let Some(far) = far.as_ref() {
      return far.intersect(r0, r1, ignore_wall);
    }

    return None;
  }

  fn raycast(&self, r0: Pos, ray_dir: Pos, ignore_wall: Option<&Wall>) -> Option<(Pos, &Wall)> {
    let q = r0;
    let p = self.splitter.a;
    let s = ray_dir;
    let r = self.splitter.b - self.splitter.a;
    // t = (q − p) × s / (r × s)
    // u = (q − p) × r / (r × s)
    let qp = q - p;
    let rs = Pos::cross(&r, &s);
    let qps = Pos::cross(&qp, &s);
    let qpr = Pos::cross(&qp, &r);

    // const ray_dir = sub_vec(r1, r0)
    // const segment_dir = sub_vec(self.splitter.b, self.splitter.a)
    // const numerator = cross_prod(sub_vec(self.splitter.a, r0), ray_dir)
    // const denominator = cross_prod(ray_dir, segment_dir)

    // const numerator_is_zero = Math.abs(numerator) < 0.00001

    // if (numerator < 0 || (numerator_is_zero && denominator > 0)) {
    let (near, far) = match qpr < 0.0 || (qpr.abs() < 0.00001 && rs > 0.0) {
      true  => (self.front.as_ref(), self.back.as_ref()),
      false => (self.back.as_ref(), self.front.as_ref()),
    };
    if let Some(near) = near.as_ref() {
      let hit = near.raycast(r0, ray_dir, ignore_wall);
      if hit.is_some() {
        return hit;
      }
    }

    // if the denominator is zero the lines are parallel
    // if (Math.abs(denominator) < 0.00001) {
    if rs.abs() < 0.00001 {
      return None;
    }

    let t = qps / rs;
    let u = qpr / rs;

    // intersection is the point on a line segment where the line divides it
    // const intersection = numerator / denominator

    // segments that are not parallel and t is in (0, 1) should be divided
    // if (0.0 < intersection && intersection < 1.0) {
    //   return add_vec(self.splitter.a, mul_vec(segment_dir, intersection))
    // }
    if u > 0.0 && 0.0 < t && t < 1.0 && !ptr::eq(&self.splitter, ignore_wall.map(ptr::from_ref).unwrap_or(ptr::null())) {
      return Some((q + (s * u), &self.splitter));
    }

    if let Some(far) = far.as_ref() {
      return far.raycast(r0, ray_dir, ignore_wall);
    }

    return None;
  }

  pub fn new(layout: &RoomLayout) -> Arc<BSP> {
    match Self::build_sub_tree(&mut rand::thread_rng(), &layout.walls) {
      Some(bsp) => Arc::from(bsp),
      None => Arc::new(BSP {
        splitter: Wall { a: Pos::ZERO, b: Pos::ZERO, damping: 0.0 },
        front: None,
        back: None,
      }),
    }
  }
  fn build_sub_tree(rng: &mut impl Rng, segments: &[Wall]) -> Option<Box<BSP>> {
    let mut best_splitter = segments.get(0)?;
    let mut best_front = Vec::with_capacity(segments.len());
    let mut best_back = Vec::with_capacity(segments.len());

    {
      let mut front = Vec::with_capacity(segments.len());
      let mut back = Vec::with_capacity(segments.len());
      for _ in 0..256 {
        let splitter = Self::bsp_split(rng, segments, &mut front, &mut back);

        let imbalance = (front.len() as isize - back.len() as isize).abs() as usize;
        let best_imbalance = (best_front.len() as isize - best_back.len() as isize).abs() as usize;
        let best_count = best_front.len() + best_back.len();
        let new_count = front.len() + back.len();
        if best_count == 0 || new_count < best_count || imbalance < best_imbalance {
          mem::swap(&mut best_front, &mut front);
          mem::swap(&mut best_back, &mut back);
          front.clear();
          back.clear();
          best_splitter = splitter;
        }
        if imbalance < 2 && new_count + 1 == segments.len() {
          break;
        }
      }
    }

    return Some(Box::new(BSP {
      splitter: best_splitter.clone(),
      front: Self::build_sub_tree(rng, &best_front),
      back: Self::build_sub_tree(rng, &best_back),
    }));
  }
  fn bsp_split<'a>(rng: &mut impl Rng, segments: &'a [Wall], front: &mut Vec<Wall>, back: &mut Vec<Wall>) -> &'a Wall {
    let splitter = &segments[rng.gen_range(0..segments.len())];
    let splitter_dir = splitter.b - splitter.a;

    for segment in segments.iter() {
      if ptr::eq(segment, splitter) {
        continue
      }
      let segment_dir = segment.b - segment.a;
      let numerator = Pos::cross(&(segment.a - splitter.a), &splitter_dir);
      let denominator = Pos::cross(&splitter_dir, &segment_dir);

      // if the denominator is zero the lines are parallel
      let denominator_is_zero = denominator.abs() < 0.00001;

      // segments are collinear if they are parallel and the numerator is zero
      let numerator_is_zero = numerator.abs() < 0.00001;

      if !denominator_is_zero {
        // intersection is the point on a line segment where the line divides it
        let intersection = numerator / denominator;

        // segments that are not parallel and t is in (0, 1) should be divided
        if 0.0 < intersection && intersection < 1.0 {
          let intersection_point = segment.a + (segment_dir * intersection);

          let mut r_segment = Wall {
            a: segment.a,
            b: intersection_point,
            damping: segment.damping,
          };

          let mut l_segment = Wall {
            a: intersection_point,
            b: segment.b,
            damping: segment.damping,
          };

          if numerator > 0.0 {
            mem::swap(&mut l_segment, &mut r_segment);
          }

          front.push(r_segment);
          back.push(l_segment);
          continue
        }
      }

      if numerator < 0.0 || (numerator_is_zero && denominator > 0.0) {
        front.push(segment.clone())
      } else {
        back.push(segment.clone())
      }
    }
    return splitter;
  }
}
