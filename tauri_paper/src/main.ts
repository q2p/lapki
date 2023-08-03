import { invoke } from "@tauri-apps/api/tauri";
import Konva from 'konva';

interface Config {
  walls: Wall[];
  radio_points: RadioPoint[];
  radio_zones: RadioZone[];
}

interface Wall {
  a: Point2d;
  b: Point2d;
  damping: number;
}

interface RadioPoint {
  pos: Point2d;
  power: number;
}

interface RadioZone {
  points: Point2d[];
}

interface Point2d {
  x: number;
  y: number;
}

async function get_config(): Promise<Config>{
  return invoke("get_config")
    .then((cfg: Config) => {return cfg} );
}

let config: Config = await get_config();


const stage = new Konva.Stage({
  container: 'container',
  width: 1000,
  height: 1000,
  draggable: true
});

const layer = new Konva.Layer();
stage.add(layer);


const WIDTH = 100;
const HEIGHT = 100;

const grid = [
  ['white', 'white'],
  ['white', 'white']
];

function checkShapes() {
  const startX = Math.floor((-stage.x() - stage.width()) / WIDTH) * WIDTH;
  const endX = Math.floor((-stage.x() + stage.width() * 2) / WIDTH) * WIDTH;

  const startY = Math.floor((-stage.y() - stage.height()) / HEIGHT) * HEIGHT;
  const endY = Math.floor((-stage.y() + stage.height() * 2) / HEIGHT) * HEIGHT;



  for (var x = startX; x < endX; x += WIDTH) {
    for (var y = startY; y < endY; y += HEIGHT) {


      const indexX = ((x / WIDTH) + grid.length * WIDTH) % grid.length;
      const indexY = ((y / HEIGHT) + grid[0].length * HEIGHT) % grid[0].length;

      //maps from 0 to 3
      const i = indexX * 2 + indexY;

      layer.add(new Konva.Rect({
        x,
        y,
        width: WIDTH,
        height: HEIGHT,
        fill: grid[indexX][indexY],
        stroke: 'gray',
        strokeWidth: 1
      }));

      let line = new Konva.Line({
        x: 0,
        y: 0,
        points: [10, 10, 10, 0],
        stroke: 'black',
        tension: 1
      });
      line.on("click", () => {
        console.log("click");
      });
      layer.add(line);
    }
  }

}

var scaleBy = 1.05;
stage.on('wheel', (e) => {
  // stop default scrolling
  e.evt.preventDefault();

  var oldScale = stage.scaleX();
  var pointer = stage.getPointerPosition();

  var mousePointTo = {
    x: (pointer.x - stage.x()) / oldScale,
    y: (pointer.y - stage.y()) / oldScale,
  };

  // how to scale? Zoom in? Or zoom out?
  let direction = e.evt.deltaY > 0 ? 1 : -1;

  // when we zoom on trackpad, e.evt.ctrlKey is true
  // in that case lets revert direction
  if (e.evt.ctrlKey) {
    direction = -direction;
  }

  var newScale = direction > 0 ? oldScale * scaleBy : oldScale / scaleBy;

  stage.scale({ x: newScale, y: newScale });

  var newPos = {
    x: pointer.x - mousePointTo.x * newScale,
    y: pointer.y - mousePointTo.y * newScale,
  };
  stage.position(newPos);
});

checkShapes();
layer.draw();

stage.on('dragend', () => {
  layer.destroyChildren();
  checkShapes();
  layer.draw();
})
