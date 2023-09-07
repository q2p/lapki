import { event } from "@tauri-apps/api";
import { invoke } from "@tauri-apps/api/tauri";
import { log } from "console";
import e from "express";
import Konva from 'konva';

// interface Config {
//   walls: Wall[];
//   radio_points: RadioPoint[];
//   radio_zones: RadioZone[];
// }

// interface Wall {
//   a: Point2d;
//   b: Point2d;
//   damping: number;
// }

// interface RadioPoint {
//   pos: Point2d;
//   power: number;
// }

// interface RadioZone {
//   points: Point2d[];
// }

// interface Point2d {
//   x: number;
//   y: number;
// }

// async function get_config(): Promise<Config>{
//   return invoke("get_config")
//     .then((cfg: Config) => {return cfg} );
// }

// let config: Config = await get_config();

// const stage = new Konva.Stage({
//   container: 'container',
//   width: 1000,
//   height: 1000,
//   draggable: true
// });

// const layer = new Konva.Layer();
// stage.add(layer);

// //50px = 1m
// const WIDTH = 60;
// const HEIGHT = 60;
// const KEY_L = 76;
// let key_l_clicked = 0;
// const KEY_S = 83;
// let key_s_clicked = 0;

// const grid = [
//   ['white', 'white'],
//   ['white', 'white']
// ];

// function drawGrid() {
//   // const startX = Math.floor((-stage.x() - stage.width()) / WIDTH) * WIDTH;
//   // const endX = Math.floor((-stage.x() + stage.width() * 2) / WIDTH) * WIDTH;

//   // const startY = Math.floor((-stage.y() - stage.height()) / HEIGHT) * HEIGHT;
//   // const endY = Math.floor((-stage.y() + stage.height() * 2) / HEIGHT) * HEIGHT;
//   let startX = 0;
//   let endX = 2500;
//   let startY = 0;
//   let endY = 2500;

//   for (var x = startX; x < endX; x += WIDTH) {
//     for (var y = startY; y < endY; y += HEIGHT) {

//       const indexX = ((x / WIDTH) + grid.length * WIDTH) % grid.length;
//       const indexY = ((y / HEIGHT) + grid[0].length * HEIGHT) % grid[0].length;

//       //maps from 0 to 3
//       const i = indexX * 2 + indexY;

//       layer.add(new Konva.Rect({
//         x,
//         y,
//         width: WIDTH,
//         height: HEIGHT,
//         fill: grid[indexX][indexY],
//         stroke: 'gray',
//         strokeWidth: 0.5
//       }));
//     }
//   }
// }

// function drawLines() {
//   for (let i = 0; i < config.walls.length; i++) {
//     let line = new Konva.Line({
//       x: 0,
//       y: 0,
//       points: [config.walls[i].a.x, config.walls[i].a.y, config.walls[i].b.x, config.walls[i].b.y,],
//       stroke: 'black',
//       tension: 1
//     });
//     // line.on("click", () => {
//     //   console.log("click");
//     // });
//     layer.add(line);
//   }
// }

// let scaleBy = 1.05;
// stage.on('wheel', (e) => {
//   // stop default scrolling
//   e.evt.preventDefault();

//   var oldScale = stage.scaleX();
//   var pointer = stage.getPointerPosition();

//   var mousePointTo = {
//     x: (pointer.x - stage.x()) / oldScale,
//     y: (pointer.y - stage.y()) / oldScale,
//   };

//   // how to scale? Zoom in? Or zoom out?
//   let direction = e.evt.deltaY > 0 ? 1 : -1;

//   // when we zoom on trackpad, e.evt.ctrlKey is true
//   // in that case lets revert direction
//   if (e.evt.ctrlKey) {
//     direction = -direction;
//   }

//   var newScale = direction > 0 ? oldScale * scaleBy : oldScale / scaleBy;

//   stage.scale({ x: newScale, y: newScale });

//   var newPos = {
//     x: pointer.x - mousePointTo.x * newScale,
//     y: pointer.y - mousePointTo.y * newScale,
//   };
//   stage.position(newPos);
// });

// drawGrid();
// drawLines();
// layer.draw();

// // let container = stage.container();
// // // make container focusable
// // container.tabIndex = 1;
// // let pos = null;
// // let mem_pos = null;
// // container.addEventListener('keydown', function (e) {
// //   if (e.keyCode === KEY_L) {
// //     if (key_l_clicked == 1) {
// //       pos = stage.getRelativePointerPosition();
// //       let line = new Konva.Line({
// //         x: 0,
// //         y: 0,
// //         points: [mem_pos.x, mem_pos.y, pos.x, pos.y],
// //         stroke: 'black',
// //         tension: 1
// //       });
// //       layer.add(line);
// //     }
// //     mem_pos = stage.getRelativePointerPosition();
// //     key_l_clicked = 1;
// //   } else if (e.keyCode == KEY_S){
// //     key_l_clicked == 0;
// //     pos = mem_pos = null;
// //   } else {
// //     return;
// //   }
// //   e.preventDefault();
// // })

// stage.on('dragend', () => {
//   layer.destroyChildren();
//   drawGrid();
//   drawLines();
//   layer.draw();
// })

// stage.on('click', function () {
//   var pos = stage.getRelativePointerPosition();
//   console.log(pos)
// });

//////////////////////////
//////////////////////////
//////////////////////////

let elements = [
  {label: 'max red', x: 100, y: 100, color: "red"},
  {label: 'min red', x: 75, y: 100, color: "red"},
  {label: 'max blue', x: 200, y: 200, color: "blue"},
  {label: 'min blue', x: 150, y: 200, color: "blue"},
  {label: 'max green', x: 150, y: 150, color: "green"},
  {label: 'min green', x: 100, y: 150, color: "green"},
];

let img = document.getElementById('rimg');
let img_wh = 8;

for (const el of elements) {
  const img = document.createElement('div');
  img.style.position = "absolute";
  img.style.left = `${el.x}px`;
  img.style.top = `${el.y}px`;
  img.style.border = "2px solid black";
  img.style.borderRadius = "100%";
  img.style.width = img_wh.toString() + "px";
  img.style.height = img_wh.toString() + "px";
  img.style.backgroundColor = el.color;

  let box = document.createElement('span');
  box.textContent = el.label;
  box.style.position = "absolute";
  box.style.opacity = "0.7";
  box.style.background = "#333";
  box.style.color = "white";
  // box.style.border = "2px solid black";
  box.style.borderRadius = "5px";
  box.style.paddingLeft = "6px";  
  box.style.paddingRight = "6px";  
  box.style.paddingTop = "2px";
  box.style.paddingBottom = "2px";
  box.style.left = (el.x + img_wh - 0.5).toString() + "px";
  box.style.top = (el.y).toString() + "px";
  document.body.appendChild(box);
  document.body.appendChild(img);
};

function push_aside() {
  let spans = document.querySelectorAll('span');
  let prev: HTMLSpanElement | null = null;
  let prev_rect: DOMRect | null = null;

  spans.forEach(function(el) {
    if (prev === null && prev_rect === null) {
      prev = el;
      prev_rect = el.getBoundingClientRect();
      return;
    }

    let rect = el.getBoundingClientRect();
    if(rect.bottom > prev_rect.top 
      && rect.right > prev_rect.left 
      && rect.top < prev_rect.bottom 
      && rect.left < prev_rect.right) {
        if (rect.bottom == prev_rect.bottom && rect.top === prev_rect.top) {
          prev!.style.top = (prev.offsetTop - prev.clientHeight).toString() + "px";
          el.style.top = (el.offsetTop + img_h).toString() + "px";
          el.style.left = (el.offsetLeft - (el.clientWidth / 2)).toString() + "px";
        }
    }
    prev = el;
    prev_rect = el.getBoundingClientRect();
  });
}

function push_aside2() {
  let spans = document.querySelectorAll('span');
  let prev: HTMLSpanElement | null = null;
  let prev_rect: DOMRect | null = null;

  // for(let i = 0; i < 100; i++) {
    spans.forEach(function(el) {
      if (prev === null && prev_rect === null) {
        prev = el;
        prev_rect = el.getBoundingClientRect();
        return;
      }
  
      let rect = el.getBoundingClientRect();
      if(rect.bottom > prev_rect.top 
        && rect.right > prev_rect.left 
        && rect.top < prev_rect.bottom 
        && rect.left < prev_rect.right) {
          let dy = Math.abs(rect.top - prev_rect.bottom);
          let dx = Math.abs(rect.left - prev_rect.right);
          prev.style.top = (prev.offsetTop - dy).toString() + "px";

          el.style.top = (el.offsetTop + img_wh).toString() + "px";
          el.style.left = (el.offsetLeft - el.clientWidth - img_wh).toString() + "px";
          prev.style.margin = "3px";
          el.style.margin = "3px";
          // prev.style.left = (prev.offsetLeft + (dx / 2)).toString() + "px";
        }
      prev = el;
      prev_rect = el.getBoundingClientRect();
    });
  }
// }

document.addEventListener('keydown', function(event) {
  if (event.code == 'KeyZ') {
    push_aside2();
  }
});

document.addEventListener('keydown', function(event) {
  if (event.code == 'KeyX') {
    push_aside();
  }
});