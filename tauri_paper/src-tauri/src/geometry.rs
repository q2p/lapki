use crate::room_state::Pos;

pub fn line_intersection(a: (Pos, Pos), b: (Pos, Pos)) -> Option<Pos> {
  let s1 = Pos::new(a.1.x - a.0.x, a.1.y - a.0.y);
  let s2 = Pos::new(b.1.x - b.0.x, b.1.y - b.0.y);

  let s = (-s1.y * (a.0.x - b.0.x) + s1.x * (a.0.y - b.0.y)) / (s1.x * s2.y - s2.x * s1.y);
  let t = ( s2.x * (a.0.y - b.0.y) - s2.y * (a.0.x - b.0.x)) / (s1.x * s2.y - s2.x * s1.y);

  if s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0 {
    return Some(Pos::new(a.0.x + (t * s1.x), a.0.y + (t * s1.y)));
  }
  return None;
}
