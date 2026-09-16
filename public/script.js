const button = document.getElementById("testButton");
const status = document.getElementById("status");

button.addEventListener("click", function() {
    status.textContent = "JavaScript is working!";
});