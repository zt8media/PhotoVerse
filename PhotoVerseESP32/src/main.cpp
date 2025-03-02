#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi Credentials
const char* ssid = "ESP32-LCD-Control";
const char* password = "12345678";

// Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create WebServer on port 80
WebServer server(80);

// Store text settings
String firstName = "";
String lastName = "";
String selectedEmoji = "";

// Define pixel-style emojis for LCD
byte heart[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000};
byte smiley[8] = {0b00000, 0b00000, 0b01010, 0b00000, 0b10001, 0b01110, 0b00000, 0b00000};
byte star[8] = {0b00000, 0b00100, 0b01110, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000};

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>ESP32 LCD Virtual Name Tag</title>
        <style>
            @import url('https://fonts.googleapis.com/css2?family=Press+Start+2P&display=swap');
            body { font-family: Arial, sans-serif; background-color: #000; color: #fff; text-align: center; }
            input, button { font-size: 1.2em; margin: 10px; padding: 10px; border-radius: 5px; border: none; }
            button { background: #00509e; color: #fff; cursor: pointer; transition: 0.3s; }
            button:hover { background: #0073e6; transform: scale(1.1); }
            .preview-box {
                display: grid;
                grid-template-columns: repeat(16, 1fr);
                grid-template-rows: repeat(2, 1fr);
                gap: 4px;
                max-width: 340px;
                margin: auto;
                padding: 10px;
                border-radius: 5px;
            }
            .char-box {
                width: 20px;
                height: 26px;
                background: black;
                color: white;
                font-family: 'Press Start 2P', sans-serif;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 1em;
                font-weight: bold;
                border-radius: 3px;
                border: 1px solid #00aaff;
                transition: 0.2s ease-in-out;
            }
        </style>
        <script>
            let tagPreview = Array(32).fill(' ');
            function updatePreview() {
                let firstName = document.getElementById('firstName').value;
                let lastName = document.getElementById('lastName').value;
                let selectedEmoji = document.querySelector('.emoji-selected')?.innerText || '';
                
                tagPreview.fill(' ');  // Ensure it starts blank
                
                for (let i = 0; i < 16; i++) {
                    tagPreview[i] = firstName[i] || ' ';
                    tagPreview[i + 16] = lastName[i] || ' ';
                }
                
                if (lastName.length < 16 && selectedEmoji) {
                    tagPreview[lastName.length + 16] = selectedEmoji;
                }
                renderPreview();
            }

            function renderPreview() {
                let previewBox = document.getElementById('preview-box');
                previewBox.innerHTML = '';
                tagPreview.forEach(char => {
                    let charDiv = document.createElement('div');
                    charDiv.classList.add('char-box');
                    charDiv.innerText = char;
                    previewBox.appendChild(charDiv);
                });
            }

            function selectEmoji(emoji) {
                document.querySelectorAll('.emoji-button').forEach(btn => btn.classList.remove('emoji-selected'));
                event.target.classList.add('emoji-selected');
                updatePreview();
            }

            function sendToMachine() {
                window.location.href = `/update?firstName=${document.getElementById('firstName').value}&lastName=${document.getElementById('lastName').value}&emoji=${document.querySelector('.emoji-selected')?.innerText || ''}`;
            }
        </script>
    </head>
    <body>
        <h2>ESP32 LCD Virtual Name Tag</h2>
        <input type="text" id="firstName" placeholder="" oninput="updatePreview()" maxlength="16">
        <br>
        <input type="text" id="lastName" placeholder="" oninput="updatePreview()" maxlength="16">
        <br>
        <div id="preview-box" class="preview-box"></div>
        <br>
        <label>Choose an Emoji:</label>
        <button class="emoji-button" onclick="selectEmoji('♥')">♥</button>
        <button class="emoji-button" onclick="selectEmoji('☺')">☺</button>
        <button class="emoji-button" onclick="selectEmoji('★')">★</button>
        <br><br>
        <button onclick="sendToMachine()">Send to Machine</button>
        <script>renderPreview();</script>
    </body>
    </html>
  )rawliteral");
}

void handleUpdate() {
  if (server.hasArg("firstName")) firstName = server.arg("firstName");
  if (server.hasArg("lastName")) lastName = server.arg("lastName");
  if (server.hasArg("emoji")) selectedEmoji = server.arg("emoji");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstName.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(lastName.substring(0, 16));
  
  if (selectedEmoji == "♥") lcd.write(byte(0));
  else if (selectedEmoji == "☺") lcd.write(byte(1));
  else if (selectedEmoji == "★") lcd.write(byte(2));
  
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, heart);
  lcd.createChar(1, smiley);
  lcd.createChar(2, star);
  
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();
}

void loop() {
  server.handleClient();
}
