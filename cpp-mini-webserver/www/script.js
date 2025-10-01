document.addEventListener("DOMContentLoaded", function() {
    document.getElementById("clickBtn").addEventListener("click", function() {
        document.getElementById("output").innerText = "Button clicked! Hello from JS.";
    });
});