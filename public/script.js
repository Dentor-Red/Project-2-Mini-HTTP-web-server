const button = document.getElementById("testButton");
const status = document.getElementById("status");
const serverStatus = document.getElementById("serverStatus");

button.addEventListener("click", function() {
    status.textContent = "JavaScript is working!";
});

fetch("/api/status")
    .then(response => response.json())
    .then(data => {
        serverStatus.textContent = data.message;
    });