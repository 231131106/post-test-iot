function listenToSensors() {
  const sensorRef = ref(db, "greenhouse/sensors");

  onValue(sensorRef, (snapshot) => {
    const d = snapshot.val() || {};

    updateSoil(d.soilMoisture);
    updateLight(d.lightLevel);
    updateMotion(d.motion);
  });
}

function updateSoil(value) {
  document.getElementById("soilMoisture").innerText = value + " %";
  const status = value < 30 ? "Kering" : value < 70 ? "Cukup" : "Basah";
  document.getElementById("soilStatus").innerText = status;
}

function updateLight(value) {
  document.getElementById("lightLevel").innerText = value + " %";
  const status = value < 30 ? "Redup" : value < 70 ? "Cukup" : "Terang";
  document.getElementById("lightStatus").innerText = status;
}

function updateMotion(value) {
  document.getElementById("motion").innerText = value
    ? "Terdeteksi"
    : "Tidak Terdeteksi";
  const status = value ? "Ada Gerakan" : "Aman";
  document.getElementById("motionStatus").innerText = status;
}
