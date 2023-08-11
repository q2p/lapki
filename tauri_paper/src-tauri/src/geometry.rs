use crate::room_state::Pos;

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
