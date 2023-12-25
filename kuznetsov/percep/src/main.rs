use std::{path::Path, time::{Instant, Duration}, ops::Sub, task::Wake};

use clap::Parser;
use minifb::{Key, Window, WindowOptions, KeyRepeat, Scale, ScaleMode};
use image::{io::Reader as ImageReader, GenericImageView};
use rand::Rng;

struct Ticker {
    next: Instant,
    lt: u64,
    rem: u64,
}
const DIST: Duration = Duration::from_millis(1000 / 25);
impl Ticker {
    pub fn new() -> Ticker {
        Ticker {
            next: Instant::now(),
            lt: 0,
            rem: 0,
        }
    }
    pub fn tick(&mut self) -> bool {
        if self.rem != self.lt {
            self.rem += 1;
            return false;
        }
        let now = Instant::now();
        let prev = self.next - DIST;
        let passed = (now - prev).as_secs_f64() / DIST.as_secs_f64();
        self.lt = ((self.lt as f64 / passed).ceil() as u64).max(1);
        if now < self.next {
            return false;
        }
        self.rem = 0;
        while self.next < now {
            self.next += DIST;
        }
        return true;
    }
}

#[derive(Parser, Debug)]
#[command()]
struct Args {
    #[arg(long)]
    img: Box<Path>,

    #[arg(long)]
    hsize: usize,

    #[arg(long)]
    hlayers: usize,

    #[arg(long)]
    lrate: f64,

    #[arg(long)]
    mbatch: usize,
}

struct Layer {
    /// Weights
    w: Box<[f64]>,
    /// Biases
    b: Box<[f64]>,
    /// Activations
    a: Box<[f64]>,

    da: Box<[f64]>,
    iters_db: usize,
    dw: Box<[f64]>,
    db: Box<[f64]>,
    dwt: Box<[f64]>,
    dbt: Box<[f64]>,

    dwm: Box<[f64]>,
    dbm: Box<[f64]>,

    acf: fn(f64) -> f64,
    der: fn(f64) -> f64,
}

impl Layer {
    fn new(inp: usize, out: usize, acf: fn(f64) -> f64, der: fn(f64) -> f64) -> Layer {
        let mut ret = Layer {
            w: vec![0.0; inp*out].into_boxed_slice(),
            b: vec![0.0; out].into_boxed_slice(),
            a: vec![0.0; out].into_boxed_slice(),
            da: vec![0.0; out].into_boxed_slice(),
            dw: vec![0.0; inp*out].into_boxed_slice(),
            dwt: vec![0.0; inp*out].into_boxed_slice(),
            dwm: vec![0.0; inp*out].into_boxed_slice(),
            db: vec![0.0; out].into_boxed_slice(),
            dbt: vec![0.0; out].into_boxed_slice(),
            dbm: vec![0.0; out].into_boxed_slice(),
            iters_db: 0,
            acf, der,
        };

        let mut rng = rand::thread_rng();
        for i in ret.w.iter_mut() {
            *i = rng.gen_range(-1.0..1.0);
        }

        return ret;
    }

    fn concat_diff(total: &mut[f64], new: &[f64]) {
        assert_eq!(total.len(), new.len());
        for (t, c) in total.iter_mut().zip(new.iter()) {
            *t += c;
        }
    }

    fn update_all(&mut self, lr: f64) {
        let alpha: f64 = lr / self.iters_db as f64;
        Self::update(&mut self.b, &mut self.dbt, &mut self.dbm, alpha);
        Self::update(&mut self.w, &mut self.dwt, &mut self.dwm, alpha);
        self.iters_db = 0;
    }

    fn update(v: &mut [f64], d: &mut [f64], m: &mut [f64], alpha: f64) {
        assert_eq!(v.len(), d.len());
        assert_eq!(v.len(), m.len());
        unsafe {
            for i in 0..v.len() {
                let v = v.get_unchecked_mut(i);
                let d = d.get_unchecked_mut(i);
                let m = m.get_unchecked_mut(i);

                *m = MOMENTUM * *m + alpha * *d;
                *v = *v + *m;
                *d = 0.0;
            }
        }
    }

    fn back_prop(&mut self, inp: &[f64], din: &mut [f64]) {
        assert_eq!(inp.len(), din.len());
        assert_eq!(inp.len() * self.a.len(), self.w.len());
        din.fill(0.0);
        unsafe {
            for i in 0..self.a.len() {
                let dz = *self.da.get_unchecked(i) * (self.der)(*self.a.get_unchecked(i));
                for j in 0..inp.len() {
                    *self.dw.get_unchecked_mut(i*inp.len()+j) = dz * *inp.get_unchecked(j);
                    *din.get_unchecked_mut(j) += *self.w.get_unchecked(i*inp.len()+j) * dz;
                }
                *self.db.get_unchecked_mut(i) = dz;
            }
        }
        self.iters_db += 1;
    }

    fn forward_prop(&mut self, inp: &[f64]) {
        assert_eq!(self.a.len() * inp.len(), self.w.len());
        for i in 0..self.a.len() {
            unsafe {
                *self.a.get_unchecked_mut(i) = *self.b.get_unchecked(i);
                for j in 0..inp.len() {
                    *self.a.get_unchecked_mut(i) += *inp.get_unchecked(j) * *self.w.get_unchecked(i*inp.len()+j);
                }
                *self.a.get_unchecked_mut(i) = (self.acf)(*self.a.get_unchecked(i));
            }
        }
    }
}

fn to_bgra(r: u8, g: u8, b: u8) -> u32 {
    u32::from_le_bytes([b, g, r, 0xFF])
}
const MOMENTUM: f64 = 0.9;
fn main() {
    let mut ticker = Ticker::new();

    let args = Args::parse();

    let img = ImageReader::open(&args.img).unwrap().decode().unwrap();
    let width = img.width() as usize;
    let height = img.height() as usize;
    let mut buffer: Vec<u32> = vec![0; (2*width) * height];

    let mut options = WindowOptions::default();
    options.scale = Scale::X4;
    options.scale_mode = ScaleMode::Stretch;
    let mut window = Window::new(
        "Test - ESC to exit",
        2*width,
        height,
        options
    ).unwrap();

    for y in 0..height {
        for x in 0..width {
            let px = img.get_pixel(x as u32, y as u32).0;
            buffer[y*2*width+x] = to_bgra(px[0], px[1], px[2]);
        }
    }

    let acf = tanh;
    let der = tanh_der;
    // let acf = relu;
    // let der = relu_der;

    let mut layers = Vec::new();
    layers.push(Layer::new(2, args.hsize, acf, der));
    for _ in 0..args.hlayers {
        layers.push(Layer::new(args.hsize, args.hsize, acf, der));
    }
    layers.push(Layer::new(args.hsize, 3, tanh, tanh_der));
    // layers.push(Layer::new(args.hsize, 3, acf, der));

    let mut rng = rand::thread_rng();

    while window.is_open() && !window.is_key_pressed(Key::Escape, KeyRepeat::No) {
        let mb_iter = (0..args.mbatch).map(|_| {
            let x = rng.gen_range(0..width);
            let y = rng.gen_range(0..height);
            (x, y)
        });
        let abs_iter = (0..width*height).map(|i| (i % width, i / width));

        for (x, y) in mb_iter {
            let ix = scale_xy(x, width);
            let iy = scale_xy(y, height);
            let [r, g, b, _] = img.get_pixel(x as u32, y as u32).0.map(scale_rgb);
            let exp = [r, g, b];

            let ins: &[f64] = &[ix, iy];
            let mut prev = ins;
            for l in layers.iter_mut() {
                l.forward_prop(prev);
                prev = &l.a;
            }

            {
                let last = layers.last_mut().unwrap();
                let r = unscale_rgb(last.a[0]);
                let g = unscale_rgb(last.a[1]);
                let b = unscale_rgb(last.a[2]);
                buffer[y*2*width+x+width] = to_bgra(r, g, b);

                for i in 0..3 {
                    last.da[i] = exp[i] - last.a[i];
                }
            }

            let mut iters = layers.iter_mut().rev();
            let mut next = iters.next().unwrap();
            for prev in iters {
                next.back_prop(&prev.a, &mut prev.da);
                next = prev;
            }
            next.back_prop(ins, &mut [0.0; 2]);

            for l in layers.iter_mut() {
                Layer::concat_diff(&mut l.dbt, &mut l.db);
                Layer::concat_diff(&mut l.dwt, &mut l.dw);
            }
        }

        for l in layers.iter_mut() {
            l.update_all(args.lrate);
        }

        if ticker.tick() {
            for y in 0..height {
                for x in 0..width {
                    let ix = scale_xy(x, width);
                    let iy = scale_xy(y, height);

                    let mut prev: &[f64] = &[ix, iy];
                    for l in layers.iter_mut() {
                        l.forward_prop(prev);
                        prev = &mut l.a;
                    }

                    {
                        let last = layers.last_mut().unwrap();
                        let r = unscale_rgb(last.a[0]);
                        let g = unscale_rgb(last.a[1]);
                        let b = unscale_rgb(last.a[2]);
                        buffer[y*2*width+x+width] = to_bgra(r, g, b);
                    }

                }
            }
        }

        window.update_with_buffer(&buffer, 2*width, height).unwrap();
    }
}

fn dump(name: &str, dbw: &[f64]) {
    const TP: usize = 5;
    let i = (dbw.len() as isize - TP as isize).max(0) as usize / 2;
    let j = (i + TP).min(dbw.len());
    println!("{name}: {:?}", &dbw[i..j]);
}

fn relu(x: f64) -> f64 {
    if x > 0.0 {
        x
    } else {
        0.1 * x
    }
}

fn relu_der(x: f64) -> f64 {
    if x > 0.0 {
        1.0
    } else {
        0.1
    }
}

fn tanh(x: f64) -> f64 {
    x.tanh()
}

fn tanh_der(x: f64) -> f64 {
    1.0 - x*x
}

fn scale_xy(v: usize, size: usize) -> f64 {
    2.0 * (v as f64) / (size as f64) - 1.0
}

fn scale_rgb(v: u8) -> f64 {
    (v as f64) / 127.5 - 1.0
}

fn unscale_rgb(v: f64) -> u8 {
    ((v + 1.0) * 127.5).round() as u8
}
