document.getElementById("loginBtn").addEventListener("click", () => {
    const email = document.getElementById("email").value;
    const pass = document.getElementById("password").value;

    signInWithEmailAndPassword(auth, email, pass)
    .then(() => {
        loginForm.style.display = "none";
        dashboard.style.display = "block";
        listenToSensors();
    })
    .catch(err => alert("Login gagal: " + err.message));
});

document.getElementById("logoutBtn").addEventListener("click", () => {
    signOut(auth).then(() => {
        dashboard.style.display = "none";
        loginForm.style.display = "block";
    });
});