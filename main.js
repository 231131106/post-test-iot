import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.0/firebase-app.js";
import {
  getAuth,
  signInWithEmailAndPassword,
  signOut,
  onAuthStateChanged,
} from "https://www.gstatic.com/firebasejs/9.22.0/firebase-auth.js";
import {
  getDatabase,
  ref,
  onValue,
} from "https://www.gstatic.com/firebasejs/9.22.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyCq-nrjFTORo72Q6CD93i7w4N13ml04Tho",
  authDomain: "post-test-iot.firebaseapp.com",
  databaseURL:
    "https://post-test-iot-default-rtdb.asia-southeast1.firebasedatabase.app/",
  projectId: "post-test-iot",
  storageBucket: "post-test-iot.appspot.com",
  messagingSenderId: "1076964864936",
  appId: "1:1076964864936:web:...",
};

// Initialisasi Firebase
const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const db = getDatabase(app);

const loginForm = document.getElementById("loginForm");
const dashboard = document.getElementById("dashboard");
const loginBtn = document.getElementById("loginBtn");
const logoutBtn = document.getElementById("logoutBtn");
const errorMsg = document.getElementById("errorMsg");

onAuthStateChanged(auth, (user) => {
  if (user) {
    console.log("User logged in:", user.email);
    loginForm.style.display = "none";
    dashboard.style.display = "block";
    listenToSensors();
  } else {
    console.log("User logged out");
    dashboard.style.display = "none";
    loginForm.style.display = "flex";
  }
});

loginBtn.addEventListener("click", () => {
  const email = document.getElementById("email").value;
  const pass = document.getElementById("password").value;
  errorMsg.innerText = "Proses masuk...";

  signInWithEmailAndPassword(auth, email, pass).catch((error) => {
    errorMsg.innerText = "Gagal: " + error.message;
  });
});

logoutBtn.addEventListener("click", () => {
  signOut(auth);
});

function listenToSensors() {
  const sensorRef = ref(db, "greenhouse/sensors");

  onValue(sensorRef, (snapshot) => {
    const d = snapshot.val();
    if (d) {
      updateSoil(d.soilMoisture);
      updateLight(d.lightlevel);
      updateMotion(d.motion);
    } else {
      console.log("Belum ada data sensor");
    }
  });
}

function updateSoil(value) {
  const el = document.getElementById("soilMoisture");
  const statusEl = document.getElementById("soilStatus");

  if (value === undefined) return;
  el.innerText = value + " %";

  statusEl.className = "status-badge";
  if (value < 40) {
    statusEl.innerText = "Kering";
    statusEl.classList.add("status-warn");
  } else {
    statusEl.innerText = "Lembab";
    statusEl.classList.add("status-good");
  }
}

function updateLight(value) {
  const el = document.getElementById("lightLevel");
  const statusEl = document.getElementById("lightStatus");

  if (value === undefined) return;
  el.innerText = value + " %";

  statusEl.className = "status-badge";
  if (value < 30) {
    statusEl.innerText = "Redup";
    statusEl.classList.add("status-warn");
  } else {
    statusEl.innerText = "Terang";
    statusEl.classList.add("status-good");
  }
}

function updateMotion(value) {
  const el = document.getElementById("motion");
  const statusEl = document.getElementById("motionStatus");

  if (value === true) {
    el.innerText = "Terdeteksi";
    statusEl.innerText = "Ada Gerakan";
    statusEl.className = "status-badge status-bad"; // Merah
  } else {
    el.innerText = "Aman";
    statusEl.innerText = "Tidak Ada";
    statusEl.className = "status-badge status-good"; // Hijau
  }
}
