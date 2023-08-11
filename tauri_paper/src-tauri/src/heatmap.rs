use std::fs::OpenOptions;
use std::io::BufWriter;
use std::num::NonZeroUsize;
use std::ops::{Deref, Add};
use std::path::Path;
use std::sync::{RwLock, Arc};

use crate::geometry::line_intersection;
use crate::random_tries::{chop_ys, RoomState2, BoundingBoxes};
use crate::room_state::{Pos, Px, RoomState, RadioZone, RadioPoint};

const STATIC_NOISE:f64 = 2.0;

// Находит силу сигнала от точки в определённом пикселе.
// n_wall - сколько стен между пикселем и точкой.
pub fn getDBm(pixel: &Pos, point: &Pos, scale_meters: f64, n_wall: usize) -> f64 {
  // Находим где находится точко относительно текущего пикселя
  let a = point.x - pixel.x;
  let b = point.y - pixel.y;
  let dist = (a * a + b * b).sqrt() * scale_meters;

  // Считаем силу сигнала в dbm
  return 31.84 + 21.50 * (dist.ln() / 10f64.ln() + 19f64 * (5f64.ln()/ 10f64.ln())) + n_wall as f64;
}

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

// Вызывается по таймеру и перерисовывает картинку
pub async fn next_image(bb: Arc<BoundingBoxes>, regular_state: Arc<RoomState>, next_guess: Arc<RoomState2>) {
  next_image1(bb.clone(), regular_state.clone(), next_guess.clone()).await;
  next_image2(bb.clone(), regular_state.clone(), next_guess.clone()).await;
}

fn length(a: Pos, b: Pos) -> f64 {
  length_sq(a, b).sqrt()
}

fn length_sq(a: Pos, b: Pos) -> f64 {
  let dx = a.x - b.x;
  let dy = a.y - b.y;
  dx*dx + dy*dy
}

fn dot(a: Pos, b: Pos) -> f64 {
  a.x*b.x + a.y+b.y
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

    inside ^= (a.y > point.y) != (b.y > point.y) && point.x < a.x + (b.x - a.x) * (point.y - a.y) / (b.y - a.y);

    i += 1;
    j = i + 1;
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
    let (a, b) = unsafe {(
      *zone.points.get_unchecked(i),
      *zone.points.get_unchecked(j)
    )};

    let dw = a - b;
    let len_sq = dw.x*dw.x+dw.y+dw.y;
    if len_sq > 0.00001 {
      let t = (dot(a - b, dw) / len_sq).clamp(0.0, 1.0);
      let projection = a + dw * t;
      let len = length(pix, projection);
      if len < min_len {
        min_len = len;
      }
    }

    i += 1;
    j = i + 1;
  }
  return min_len;
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn next_image1(bb: Arc<BoundingBoxes>, regular_state: Arc<RoomState>, next_guess: Arc<RoomState2>) {
  do_image(bb, regular_state.clone(), "../rimg1.png", Arc::new(move |pixel| {
    // Рассматриваем каждую точку доступа

    let mut dBm = f64::MAX;
    for point in regular_state.radio_points.iter() {
      // println!("{:?}", point.pos);
      let mut count_wall = 0;

      // Смотрим сколько стен пересекаются с лучём видимости.
      for wall in regular_state.walls.iter() {
        if line_intersection((pixel, point.pos), (wall.a, wall.b)).is_some() {
          count_wall += 1;
        }
      }
      // Считаем силу сигнала и запоминаем самый сильный
      let dBc = getDBm(&pixel, &point.pos, 1.0, count_wall) / 1000.0;
      if dBc < dBm {
        dBm = dBc;
      }
    }

    // println!("{dBm}");

    // Обрезаем верхний и нижние участки выходящие за допустимые границы
    let mut s = scale_dbs(dBm);
    // println!("{dBm}");
    // let mut s = ((dBm-0.35)*100.0).clamp(0.0, 1.0);
    // Делаем плавные переходы ступенчатыми (40 ступеней)
    s = (s * 40.0) as u8 as f64 / 40.0;
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
    let mut r = 0;
    let mut g = 0;
    let mut b = 0;
    for i in 0..(GRAD.len()-1) {
      let g0 = &GRAD[i+0];
      let g1 = &GRAD[i+1];
      // Проверяем, попадаем ли мы в нужный диапазон
      if s >= g0.t && s <= g1.t {
        let t = (s - g0.t) / (g1.t - g0.t);
        r = (g0.r as f64 * (1.0 - t) + g1.r as f64 * t) as u8;
        g = (g0.g as f64 * (1.0 - t) + g1.g as f64 * t) as u8;
        b = (g0.b as f64 * (1.0 - t) + g1.b as f64 * t) as u8;
        break;
      }
    }
    return (r, g, b);
  })).await;
}

// Задаём область силы сигнала которую сможем отобразить
// Если сигнал выходит за рамки [0.20, 0.31], то он обрезается.
const dbmin: f64 = 0.31;
const dbmax: f64 = 0.368;
// Чтобы цвета было легче различать, мы должны привратить децибеллы,
// в другую величину, которая более линейна, чтобы цвета были расположены
// более равномерно
const dbpow: f64 = 4.0;
fn scale_dbs(dBm: f64) -> f64 {
  ((dBm - dbmin) / (dbmax - dbmin))
    .clamp(0.0, 1.0)
    // Изменяем кривизну интенсивностей сигнала
    .powf(dbpow)
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn next_image2(bb: Arc<BoundingBoxes>, regular_state: Arc<RoomState>, next_guess: Arc<RoomState2>) {
  do_image(bb, regular_state.clone(), "../rimg2.png", Arc::new(move |pix| {
    // Рассматриваем каждую точку доступа
    let dbms: Vec<f64> = regular_state.radio_points.iter().map(|point| dbm_after_walls(&regular_state, pix, point)).collect();

    const P: f64 = 2.0;

    let mut A = [0.0, 0.0, 0.0];
    let mut B = [0.0, 0.0, 0.0];
    for zone in regular_state.radio_zones.iter() {
      let zone_rgb = [zone.r, zone.g, zone.b];
      let d = distance_to_zone(pix, zone).powf(P);
      let signal = dbms[zone.desired_point_id];
      let all_signals = dbms.iter().sum::<f64>() + STATIC_NOISE;
      let noise = all_signals - signal;
      let sig_to_all = signal / all_signals;
      let noise_to_all = noise / all_signals;
      let sinr = signal / noise;

      let sinr_01 = scale_dbs(sinr);

      for i in 0..3 {
        // let color = 255.0 * noise_to_all + zone_rgb[i] as f64 * sig_to_all;
        // let color = 255.0 * sinr_01 + zone_rgb[i] as f64 * sinr_01;
        let color = zone_rgb[i] as f64;
        A[i] += color / d;
        B[i] += 1.0 / d;
      }
    }
    return (
      (A[0] / B[0]).clamp(0.0, 255.0) as u8,
      (A[1] / B[1]).clamp(0.0, 255.0) as u8,
      (A[2] / B[2]).clamp(0.0, 255.0) as u8,
    )
  })).await;
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn do_image<'a, F>(bb: Arc<BoundingBoxes>, regular_state: Arc<RoomState>, png_path: &'static str, func: Arc<F>)
where F: Fn(Pos) -> (u8, u8, u8) + Send + Sync + 'static
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

          // Закрашеваем пиксель
          unsafe {
            let offset = state.as_ptr().add((y * bb.res.0 + x) * 3) as *mut u8;
            core::ptr::write(offset.add(0), r);
            core::ptr::write(offset.add(1), g);
            core::ptr::write(offset.add(2), b);
          }
        }
      }
    }));
  }
  for queued in queued {
    queued.await.unwrap();
  }

  let mut state = state_arc.write().unwrap();
  let state = &mut *state;

  paint_walls(&bb, &regular_state, state);

  save_image(bb.res, state, png_path);
}

fn dbm_after_walls(regular_state: &RoomState, pixel: Pos, point: &RadioPoint) -> f64 {
  let mut count_wall = 0;

  // Смотрим сколько стен пересекаются с лучём видимости.
  for wall in regular_state.walls.iter() {
    if line_intersection((pixel, point.pos), (wall.a, wall.b)).is_some() {
      count_wall += 1;
    }
  }
  return getDBm(&pixel, &point.pos, 1.0, count_wall) / 1000.0;
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

pub fn meter_to_pix(bb: &BoundingBoxes, p: Pos) -> Pos {
  Pos::new(
    (p.x as f64 - bb.min.x) * bb.res.0 as f64 / bb.wh.x,
    (p.y as f64 - bb.min.y) * bb.res.1 as f64 / bb.wh.y,
  )
}

pub fn pix_to_meter(bb: &BoundingBoxes, p: Px) -> Pos {
  Pos::new(
    bb.min.x + p.x as f64 * bb.wh.x / bb.res.0 as f64,
    bb.min.y + p.y as f64 * bb.wh.y / bb.res.1 as f64,
  )
}

fn paint_walls(bb: &BoundingBoxes, regular_state: &RoomState, scene: &mut [u8]) {
  // Рисуем стены поверх изображения интенсивностей
  for w in &regular_state.walls {
    let a = meter_to_pix(bb, w.a);
    let b = meter_to_pix(bb, w.b);
    let ax = a.x as isize;
    let ay = a.y as isize;
    let bx = b.x as isize;
    let by = b.y as isize;
    // tline(&mut state.scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00, 4);
    dline(bb.res.0, scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00);
    // bline(&mut state.scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00);
  }
}

fn dline(width: usize, dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8) {
  bline(width, dest, Px::new(p0.x, p0.y), Px::new(p1.x, p1.y), r, g, b);
  bline(width, dest, Px::new(p0.x, p0.y - 1), Px::new(p1.x, p1.y - 1), r, g, b);
  bline(width, dest, Px::new(p0.x, p0.y + 1), Px::new(p1.x, p1.y + 1), r, g, b);
  bline(width, dest, Px::new(p0.x - 1, p0.y), Px::new(p1.x - 1, p1.y), r, g ,b);
  bline(width, dest, Px::new(p0.x + 1, p0.y), Px::new(p1.x + 1, p1.y), r, g ,b);
}


fn tline(width: usize, dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8, thickness: u8) {
  bline(width, dest, p0, p1, r, g, b);
  let dy = (p1.y - p0.y).abs();
  let dx = (p1.x - p0.x).abs();
  let wz = (thickness - 1) as isize * ((dx * dx + dy * dy) as f64).sqrt() as isize;
  if dx > dy {
    let wy = wz / (2 * dx);
    for i in 0..wy {
      bline(width, dest, Px::new(p0.x, p0.y - i), Px::new(p1.x, p1.y - i), r, g, b);
      bline(width, dest, Px::new(p0.x, p0.y + i), Px::new(p1.x, p1.y + i), r, g, b);
    }
  } else {
    let wx = wz / (2 * dy);
    for i in 0..wx {
      bline(width, dest, Px::new(p0.x - i, p0.y), Px::new(p1.x - i, p1.y), r, g ,b);
      bline(width, dest, Px::new(p0.x + i, p0.y), Px::new(p1.x + i, p1.y), r, g ,b);
    }
  }
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
fn bline(width: usize, dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8) {
  let dx = (p1.x - p0.x).abs();
  let dy = (p1.y - p0.y).abs();
  let xinc = if p0.x < p1.x { 1 } else { -1 };
  let yinc = if p0.y < p1.y { 1 } else { -1 };
  let mut x = p0.x;
  let mut y = p0.y;
  put(width, dest, x, y, r, g, b);
  if dx >= dy {
    let mut e = (2 * dy) - dx;
    while x != p1.x {
      if e < 0 {
        e += 2 * dy;
      } else {
        e += 2 * (dy - dx);
        y += yinc;
      }
      x += xinc;
      put(width, dest, x, y, r, g, b);
    }
  } else {
    let mut e = (2 * dx) - dy;
    while y != p1.y {
      if e < 0 {
        e += 2 * dx;
      } else {
        e += 2 * (dx - dy);
        x += xinc;
      }
      y += yinc;
      put(width, dest, x, y, r, g, b);
    }
  }
}