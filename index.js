import * as THREE from "three";

let scene, camera, renderer, cube;
let infoDiv;

function initThree() {
  scene = new THREE.Scene();
  camera = new THREE.PerspectiveCamera(70, window.innerWidth/window.innerHeight, 0.1, 1000);
  camera.position.z = 2;

  renderer = new THREE.WebGLRenderer({antialias:true});
  renderer.setSize(window.innerWidth, window.innerHeight);
  document.body.appendChild(renderer.domElement);

  // Cubo básico
  const geometry = new THREE.BoxGeometry(1,1,1);
  const material = new THREE.MeshNormalMaterial();
  cube = new THREE.Mesh(geometry, material);
  scene.add(cube);

  window.addEventListener('resize', onWindowResize);
}

function onWindowResize(){
  camera.aspect = window.innerWidth/window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate(){
  requestAnimationFrame(animate);
  renderer.render(scene, camera);
}

function connectWebSocket(){
  // SUPONIENDO que la ESP32 obtiene IP 192.168.142.5 en tu móvil
  // Míralo en el Serial de la ESP32 (WiFi.localIP()) y ajusta:
  let ipESP = "192.168.142.1";

  let wsUrl = `${protocol}://${ipESP}/ws`;
  let socket = new WebSocket(wsUrl);

  socket.onopen = () => {
    infoDiv.innerText = "WebSocket conectado";
  };

  socket.onmessage = (evt) => {
    try {
      let data = JSON.parse(evt.data);
      // BNO085 => (qw, qx, qy, qz)
      // Three.js => (x, y, z, w)
      // => .set(qx, qy, qz, qw)
      cube.quaternion.set(data.qx, data.qy, data.qz, data.qw);

      infoDiv.innerText = 
        `qw=${data.qw.toFixed(2)}\n`+
        `qx=${data.qx.toFixed(2)}\n`+
        `qy=${data.qy.toFixed(2)}\n`+
        `qz=${data.qz.toFixed(2)}\n`;
    } catch(e) {
      console.error("Error parseando data:", e);
    }
  };

  socket.onclose = () => {
    infoDiv.innerText = "WebSocket cerrado. Reintentando en 2s...";
    setTimeout(connectWebSocket, 2000);
  };
}

// Lanzar todo
window.addEventListener('load', ()=>{
  infoDiv = document.getElementById('info');
  initThree();
  animate();
  connectWebSocket();
});
