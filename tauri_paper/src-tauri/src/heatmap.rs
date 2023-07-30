use std::fs::OpenOptions;
use std::io::BufWriter;
use std::num::NonZeroUsize;
use std::path::Path;
use std::sync::{RwLock, Arc};
use std::time::{Instant, Duration};

// Разрешение изображения
const map_size: usize = 8*64;

// двухмерный вектор (x, y)
#[derive(Copy, Clone)]
struct Pos {
  x: f64,
  y: f64,
}

impl Pos {
  pub fn new(x: f64, y: f64) -> Pos {
    Pos { x, y }
  }
}

#[derive(Copy, Clone)]
struct Px {
  x: isize,
  y: isize,
}

impl Px {
  pub fn new(x: isize, y: isize) -> Px {
    Px { x, y }
  }
}

// Находит силу сигнала от точки в определённом пикселе.
// n_wall - сколько стен между пикселем и точкой.
fn getDBm(pixel: &Pos, point: &Pos, scale_meters: f64, n_wall: usize) -> f64 {
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

// Проверка пересечения двух отрезков. (p0, p1) и (p2, p3).
// Точка пересечения записывается в intersection
fn line_intersection(p0: &Pos, p1: &Pos, p2: &Pos, p3: &Pos) -> Option<Pos> {
  let s1 = Pos::new(p1.x - p0.x, p1.y - p0.y);
  let s2 = Pos::new(p3.x - p2.x, p3.y - p2.y);

  let s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (s1.x * s2.y - s2.x * s1.y);
  let t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (s1.x * s2.y - s2.x * s1.y);

  if s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0 {
    return Some(Pos::new(p0.x + (t * s1.x), p0.y + (t * s1.y)));
  }
  return None;
}

struct State {
  m_walls: Vec<(Pos, Pos)>,
  points: Vec<Pos>,
  scene: Box<[u8]>,
}

// Вызывается по таймеру и перерисовывает картинку
pub async fn next_image() {
  let start = Instant::now();
  let path = Path::new("../rimg.png");
  let concurrency = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let state_arc = Arc::new(RwLock::new(State {
    m_walls: Vec::new(),
    points: Vec::new(),
    scene: vec![0u8; map_size*map_size*3].into_boxed_slice(),
  }));

  loop {
    // Текущее время (нужно для анимации)
    let now = Instant::now().saturating_duration_since(start).as_secs_f64() / 20f64;

    // Расставляем точки доступа и стены
    {
      let mut state = state_arc.write().unwrap();
      state.m_walls.clear();
      state.points.clear();

      // Некоторые стены фиксированны
      state.m_walls.push((Pos::new(0.46, 0.46), Pos::new(0.54, 0.46) ));
      state.m_walls.push((Pos::new(0.70, 0.60), Pos::new(0.70, 0.35) ));
      state.m_walls.push((Pos::new(0.60, 0.65), Pos::new(0.40, 0.50) ));
      state.m_walls.push((Pos::new(0.46, 0.48), Pos::new(0.54, 0.48) ));
      // Некоторые крутятся по кругу
      let time0 = 6.66 * std::f64::consts::PI * (now - 0.1);
      let time1 = 6.66 * std::f64::consts::PI * (now + 0.1);
      state.m_walls.push((
        Pos::new(0.5+0.25*time0.sin(), 0.5+0.25*time0.cos()),
        Pos::new(0.5+0.25*time1.sin(), 0.5+0.25*time1.cos())
      ));

      // Расставляем точки доступа (тоже крутятся)
      let time = 8.0 * std::f64::consts::PI * now;
      state.points.push(Pos::new(0.5+0.02*time.sin(), 0.5+0.02*time.cos()));
      // points.emplace_back(Vec(0.5+0.25*qSin(2*M_PI*time), 0.5+0.25*qCos(2*M_PI*time)));
    }

    // Задаём область силы сигнала которую сможем отобразить
    // Если сигнал выходит за рамки [0.20, 0.31], то он обрезается.
    const dbmin: f64 = 0.20;
    const dbmax: f64 = 0.31;
    // Чтобы цвета было легче различать, мы должны привратить децибеллы,
    // в другую величину, которая более линейна, чтобы цвета были расположены
    // более равномерно
    const dbpow: f64 = 1.0;

    // Перебираем все пиксели на холсте

    {
      let mut queued = Vec::with_capacity(concurrency);
      let mut y_from = 0;

      for i in 0..queued.capacity() {
        let y_to = map_size * (i+1) / queued.capacity();

        let state = state_arc.clone();

        queued.push(tauri::async_runtime::spawn(async move {
          let state = state.read().unwrap();
          // Перебираем все пиксели на холсте
          for y in y_from..y_to {
            for x in 0..map_size {
              let pixel = Pos::new(x as f64 / map_size as f64 , y as f64 / map_size as f64);
              let mut dBm = 1000f64;
              // Рассматриваем каждую точку доступа
              for point in &state.points {
                let mut count_wall = 0;

                // Смотрим сколько стен пересекаются с лучём видимости.
                for (w0, w1) in &state.m_walls {
                  if line_intersection(&pixel, point, w0, w1).is_some() {
                    count_wall += 1;
                  }
                }
                // Считаем силу сигнала и запоминаем самый сильный
                let dBc = getDBm(&pixel, point, 1.0, count_wall) / 1000.0;
                if dBc < dBm  {
                  dBm = dBc;
                }
              }

              // Обрезаем верхний и нижние участки выходящие за допустимые границы
              let mut s = ((dBm - dbmin) / (dbmax-dbmin))
                .clamp(0.0, 1.0)
                // Изменяем кривизну интенсивностей сигнала
                .powf(dbpow);
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
              // Закрашеваем пиксель
              unsafe {
                let offset = state.scene.as_ptr().add((y * map_size + x) * 3) as *mut u8;
                core::ptr::write(offset.add(0), r);
                core::ptr::write(offset.add(1), g);
                core::ptr::write(offset.add(2), b);
              }
            }
          }
        }));

        y_from = y_to;
      }
      for queued in queued {
        queued.await.unwrap();
      }
    }

    {
      let mut state = state_arc.write().unwrap();
      let state = &mut *state;

      // Рисуем стены поверх изображения интенсивностей
      for (a, b) in &state.m_walls {
        let ax = (a.x * map_size as f64) as isize;
        let ay = (a.y * map_size as f64) as isize;
        let bx = (b.x * map_size as f64) as isize;
        let by = (b.y * map_size as f64) as isize;
        // tline(&mut state.scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00, 4);
        dline(&mut state.scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00);
        // bline(&mut state.scene, Px::new(ax, ay), Px::new(bx, by), 0x00, 0x00, 0x00);
      }

      println!("a1: {}", path.to_str().unwrap());
      let file = OpenOptions::new().create(true).write(true).truncate(true).open(&path).unwrap();
      let ref mut w = BufWriter::new(file);

      let mut encoder = png::Encoder::new(w, map_size as u32, map_size as u32);
      encoder.set_color(png::ColorType::Rgb);
      encoder.set_depth(png::BitDepth::Eight);
      encoder.set_compression(png::Compression::Best);
      encoder.set_source_gamma(png::ScaledFloat::from_scaled(45455)); // 1.0 / 2.2, scaled by 100000
      let source_chromaticities = png::SourceChromaticities::new(     // Using unscaled instantiation here
          (0.31270, 0.32900),
          (0.64000, 0.33000),
          (0.30000, 0.60000),
          (0.15000, 0.06000)
      );
      encoder.set_source_chromaticities(source_chromaticities);
      let mut writer = encoder.write_header().unwrap();

      writer.write_image_data(&state.scene).unwrap();
      writer.finish().unwrap();
    }

    tokio::time::sleep(Duration::from_millis(100)).await;
  }
}

fn dline(dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8) {
  bline(dest, Px::new(p0.x, p0.y), Px::new(p1.x, p1.y), r, g, b);
  bline(dest, Px::new(p0.x, p0.y - 1), Px::new(p1.x, p1.y - 1), r, g, b);
  bline(dest, Px::new(p0.x, p0.y + 1), Px::new(p1.x, p1.y + 1), r, g, b);
  bline(dest, Px::new(p0.x - 1, p0.y), Px::new(p1.x - 1, p1.y), r, g ,b);
  bline(dest, Px::new(p0.x + 1, p0.y), Px::new(p1.x + 1, p1.y), r, g ,b);
}


fn tline(dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8, thickness: u8) {
  bline(dest, p0, p1, r, g, b);
  let dy = (p1.y - p0.y).abs();
  let dx = (p1.x - p0.x).abs();
  let wz = (thickness - 1) as isize * ((dx * dx + dy * dy) as f64).sqrt() as isize;
  if dx > dy {
    let wy = wz / (2 * dx);
    for i in 0..wy {
      bline(dest, Px::new(p0.x, p0.y - i), Px::new(p1.x, p1.y - i), r, g, b);
      bline(dest, Px::new(p0.x, p0.y + i), Px::new(p1.x, p1.y + i), r, g, b);
    }
  } else {
    let wx = wz / (2 * dy);
    for i in 0..wx {
      bline(dest, Px::new(p0.x - i, p0.y), Px::new(p1.x - i, p1.y), r, g ,b);
      bline(dest, Px::new(p0.x + i, p0.y), Px::new(p1.x + i, p1.y), r, g ,b);
    }
  }
}
#[inline(always)]
fn put(dest: &mut [u8], x: isize, y: isize, r: u8, g: u8, b: u8) {
  let offset = (y as usize * map_size + x as usize) * 3;
  unsafe {
    *dest.get_unchecked_mut(offset  ) = r;
    *dest.get_unchecked_mut(offset+1) = g;
    *dest.get_unchecked_mut(offset+2) = b;
  }
  // dest[offset  ] = r;
  // dest[offset+1] = g;
  // dest[offset+2] = b;
}
fn bline(dest: &mut [u8], p0: Px, p1: Px, r: u8, g: u8, b: u8) {
  let dx = (p1.x - p0.x).abs();
  let dy = (p1.y - p0.y).abs();
  let xinc = if p0.x < p1.x { 1 } else { -1 };
  let yinc = if p0.y < p1.y { 1 } else { -1 };
  let mut x = p0.x;
  let mut y = p0.y;
  put(dest, x, y, r, g, b);
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
      put(dest, x, y, r, g, b);
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
      put(dest, x, y, r, g, b);
    }
  }
}