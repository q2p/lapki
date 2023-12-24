use minifb::{Key, Window, WindowOptions};
use image::{io::Reader as ImageReader, GenericImageView};
use rand::Rng;

fn to_bgra(r: u8, g: u8, b: u8) -> u32 {
    u32::from_le_bytes([b, g, r, 0xFF])
}
fn main() {
    let img = ImageReader::open("src.png").unwrap().decode().unwrap();
    let width = img.width() as usize;
    let height = img.height() as usize;
    let mut buffer: Vec<u32> = vec![0; (2*width) * height];

    let mut window = Window::new(
        "Test - ESC to exit",
        2*width,
        height,
        WindowOptions::default(),
    ).unwrap();

    // Limit to max ~60 fps update rate
    window.limit_update_rate(Some(std::time::Duration::from_micros(16600)));

    for y in 0..height {
        for x in 0..width {
            let px = img.get_pixel(x as u32, y as u32).0;
            buffer[y*2*width+x] = to_bgra(px[0], px[1], px[2]);
        }
    }

    const INS: usize = 2;
    const HLS: usize = 64;
    let mut w0 = vec![0.0; INS*HLS];
    let mut w1 = vec![0.0; HLS*HLS];
    let mut w2 = vec![0.0; HLS*3];

    let mut b0 = vec![0.0; HLS];
    let mut b1 = vec![0.0; HLS];
    let mut b2 = vec![0.0; 3];

    let mut a0 = vec![0.0; HLS];
    let mut a1 = vec![0.0; HLS];
    let mut a2 = vec![0.0; 3];

    let mut dins = vec![0.0; INS];
    let mut d0 = vec![0.0; HLS];
    let mut d1 = vec![0.0; HLS];
    let mut d2 = vec![0.0; 3];

    fill_rng(&mut w0);
    fill_rng(&mut w1);
    fill_rng(&mut w2);

    fill_rng(&mut b0);
    fill_rng(&mut b1);

    fill_rng(&mut a0);
    fill_rng(&mut a1);

    let mut iters_db = 0;
    let mut dbw0 = vec![0.0; w0.len()];
    let mut dbw1 = vec![0.0; w1.len()];
    let mut dbw2 = vec![0.0; w2.len()];
    let mut dbb0 = vec![0.0; b0.len()];
    let mut dbb1 = vec![0.0; b1.len()];
    let mut dbb2 = vec![0.0; b2.len()];
    let mut dbw0t = vec![0.0; w0.len()];
    let mut dbw1t = vec![0.0; w1.len()];
    let mut dbw2t = vec![0.0; w2.len()];
    let mut dbb0t = vec![0.0; b0.len()];
    let mut dbb1t = vec![0.0; b1.len()];
    let mut dbb2t = vec![0.0; b2.len()];

    while window.is_open() && !window.is_key_down(Key::Escape) {
        for y in 0..height {
            for x in 0..width {
                let ix = scale_xy(x, width);
                let iy = scale_xy(y, height);
                let ins = vec![ix, iy];
                let [r, g, b, _] = img.get_pixel(x as u32, y as u32).0.map(scale_rgb);
                let exp = [r, g, b];

                forward_prop(&ins, &mut a0, &b0, &mut w0, relu);
                forward_prop(&a0, &mut a1, &b1, &mut w1, relu);
                forward_prop(&a1, &mut a2, &b2, &mut w2, tanh);

                let r = unscale_rgb(a2[0]);
                let g = unscale_rgb(a2[1]);
                let b = unscale_rgb(a2[2]);
                buffer[y*2*width+x+width] = to_bgra(r, g, b);

                for i in 0..3 {
                    d2[i] = exp[i] - a2[i];
                }

                back_prop(
                    &a1, &a2,
                    &w2,
                    &mut dbb2, &mut dbw2,
                    &mut d1, &mut d2,
                    tanh_der
                );

                back_prop(
                    &a0, &a1,
                    &w1,
                    &mut dbb1, &mut dbw1,
                    &mut d0, &mut d1,
                    relu_der
                );

                back_prop(
                    &ins, &a0,
                    &w0,
                    &mut dbb0, &mut dbw0,
                    &mut dins, &mut d0,
                    relu_der
                );

                concat_diff(&mut dbb0t, &dbb0);
                concat_diff(&mut dbb1t, &dbb1);
                concat_diff(&mut dbb2t, &dbb2);
                concat_diff(&mut dbw0t, &dbw0);
                concat_diff(&mut dbw1t, &dbw1);
                concat_diff(&mut dbw2t, &dbw2);

                iters_db += 1;
            }
        }

        dump("dbb1", &dbb1t);
        dump("dbw1", &dbw1t);
        dump("b1", &b1);
        dump("w1", &w1);

        window.update_with_buffer(&buffer, 2*width, height).unwrap();

        update(&mut w0, &mut dbw0t, iters_db);
        update(&mut w1, &mut dbw1t, iters_db);
        update(&mut w2, &mut dbw2t, iters_db);
        update(&mut b0, &mut dbb0t, iters_db);
        update(&mut b1, &mut dbb1t, iters_db);
        update(&mut b2, &mut dbb2t, iters_db);
    }
}

fn dump(name: &str, dbw: &[f64]) {
    const TP: usize = 5;
    let i = (dbw.len() - TP).max(0) / 2;
    let j = (i + TP).min(dbw.len());
    println!("{name}: {:?}", &dbw[i..j]);
}

fn concat_diff(total: &mut[f64], new: &[f64]) {
    assert_eq!(total.len(), new.len());
    for (t, c) in total.iter_mut().zip(new.iter()) {
        *t += c;
    }
}

fn update(v: &mut [f64], d: &mut [f64], iters: usize) {
    const ALPHA: f64 = 0.1;
    for (v, d) in v.iter_mut().zip(d.iter_mut()) {
        *v += ALPHA * *d / iters as f64;
        *d = 0.0;
    }
}

fn back_prop(inp: &[f64], out: &[f64], w: &[f64], db: &mut [f64], dw: &mut [f64], din: &mut [f64], dout: &[f64], deriv: fn(f64) -> f64) {
    din.fill(0.0);
    for i in 0..out.len() {
        let dz = (dout[i]) * (deriv)(out[i]);
        for j in 0..inp.len() {
            dw[i*inp.len()+j] = dz * inp[j];
            din[j] += w[i*inp.len()+j] * dz;
        }
        db[i] = dz;
    }
}

fn forward_prop(inp: &[f64], out: &mut [f64], b: &[f64], w: &[f64], func: fn(f64) -> f64) {
    for i in 0..out.len() {
        out[i] = b[i];
        for j in 0..inp.len() {
            out[i] += inp[j] * w[i*inp.len()+j];
        }
        out[i] = (func)(out[i]);
    }
}

fn fill_rng(w: &mut [f64]) {
    let mut rng = rand::thread_rng();
    for i in w.iter_mut() {
        *i = rng.gen_range(-1.0..1.0);
    }
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
    1.0 - x.tanh().powi(2)
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
