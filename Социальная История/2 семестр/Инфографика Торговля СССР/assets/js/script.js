const canvasWrapper = document.getElementById("canvasWrapper");
const canvas = document.querySelector(".canvas");
const maximizeBtn = document.getElementById("maximizeBtn");
const minimizeBtn = document.getElementById("minimizeBtn");
const scaleBtn = document.getElementById("scaleBtn");

let isDown = false;
let startX;
let startY;
let scrollLeft;
let scrollTop;
let scale = 1;
const minScale = 0.5;
const maxScale = 2;

function correctFloat(number) {
  return Math.round(number * 100) / 100;
}

function transformCanvas(canvas, scale) {
  canvas.style.transform = `scale(${scale})`;
  updateScaleBtnText(scaleBtn, scale);
}

function updateScaleBtnText(scaleBtn, scale) {
  scaleBtn.innerHTML = `${Math.round(scale * 100)}%`;
}

canvasWrapper.addEventListener("mousedown", (e) => {
  isDown = true;
  canvasWrapper.classList.add("active");
  startX = e.pageX - canvasWrapper.offsetLeft;
  startY = e.pageY - canvasWrapper.offsetTop;
  scrollLeft = canvasWrapper.scrollLeft;
  scrollTop = canvasWrapper.scrollTop;
});

canvasWrapper.addEventListener("mouseleave", () => {
  isDown = false;
});

canvasWrapper.addEventListener("mouseup", () => {
  isDown = false;
});

canvasWrapper.addEventListener("mousemove", (e) => {
  if (!isDown) return;
  e.preventDefault();
  const x = e.pageX - canvasWrapper.offsetLeft;
  const y = e.pageY - canvasWrapper.offsetTop;
  const walkX = x - startX;
  const walkY = y - startY;
  canvasWrapper.scrollLeft = scrollLeft - walkX;
  canvasWrapper.scrollTop = scrollTop - walkY;
});

canvasWrapper.addEventListener("wheel", (e) => {
  if (!e.ctrlKey) return;
  e.preventDefault();

  const delta = e.deltaY > 0 ? -0.1 : 0.1;
  scale = correctFloat(Math.min(maxScale, Math.max(minScale, scale + delta)));

  transformCanvas(canvas, scale);
});

maximizeBtn.addEventListener("click", () => {
  scale += scale < maxScale ? 0.1 : 0;
  scale = correctFloat(scale);
  transformCanvas(canvas, scale);
});

minimizeBtn.addEventListener("click", () => {
  scale -= scale > minScale ? 0.1 : 0;
  scale = correctFloat(scale);
  transformCanvas(canvas, scale);
});

scaleBtn.addEventListener("click", () => {
  scale = 1;
  transformCanvas(canvas, scale);
  updateScaleBtnText(scaleBtn, scale);
});
