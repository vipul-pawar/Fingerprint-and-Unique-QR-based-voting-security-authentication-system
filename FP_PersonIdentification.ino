#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <HTTPClient.h>

// 🔷 WiFi credentials
const char* ssid = "ESP32_QR_SYSTEM";
const char* password = "12345678";

// Google Script URL
String scriptURL = "https://script.google.com/macros/s/AKfycbzY2jVLlR8Apm6hnHyC1VMxfvSn9kASynQOK2_QQXJyGLJKTth-q4RUa_mbJZqp01iT/exec";

WebServer server(80);

// 🔷 Fingerprint setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// 🔷 Store QR ID
int qrID = -1;

// 🔷 Homepage
void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>
body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #000;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
}

.card {
  background: rgba(255, 255, 255, 0.08);
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  padding: 22px;
  border-radius: 16px;
  width: 300px;
  text-align: center;
  box-shadow: 0 0 25px rgba(99,102,241,0.25),
              0 10px 30px rgba(0,0,0,0.6);
  border: 1px solid rgba(255,255,255,0.2);
  color: white;
}

h2 {
  color: #fff;
}

p {
  color: rgba(255,255,255,0.85);
  font-size: 14px;
}

.status {
  background: rgba(255,255,255,0.1);
  padding: 10px;
  border-radius: 10px;
  margin: 15px 0;
  color: #fff;
  font-weight: 500;
}

button {
  width: 100%;
  padding: 12px;
  background: rgba(99,102,241,0.7);
  color: white;
  border: 1px solid rgba(255,255,255,0.2);
  border-radius: 12px;
  font-size: 16px;
  backdrop-filter: blur(5px);
  transition: 0.3s;
}

button:hover {
  background: rgba(99,102,241,1);
  transform: scale(1.05);
}

#result {
  margin-top: 15px;
  font-weight: 500;
  color: #fff;
}
</style>
</head>

<body>

<div class="card">
  <h2>Identity System</h2>
  <p>Scan QR Then Verify</p>

  <div class="status">
    QR ID: )rawliteral" + String(qrID == -1 ? "Not Scanned" : String(qrID)) + R"rawliteral(
  </div>

  <button onclick="verify()">Verify Fingerprint</button>

  <p id="result"></p>
</div>

<script>
function verify(){
  document.getElementById("result").innerHTML = "⏳ Scanning...";

  fetch('/verify')
  .then(res => res.text())
  .then(data => {
    document.getElementById("result").innerHTML = data;
  });
}
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

// 🔷 QR Page
void handleQR() {
  String qrData = server.arg("qr");

  Serial.print("QR Received: ");
  Serial.println(qrData);

  qrID = qrData.toInt();

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>
body {
  margin: 0;
  font-family: Arial;
  background: #000;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
}

.card {
  background: rgba(255, 255, 255, 0.08);
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  padding: 25px;
  border-radius: 16px;
  width: 280px;
  text-align: center;
  box-shadow: 0 0 25px rgba(34,197,94,0.25),
              0 10px 30px rgba(0,0,0,0.6);
  border: 1px solid rgba(255,255,255,0.2);
  color: white;
}

.success {
  color: #22c55e;
  font-size: 18px;
  font-weight: bold;
}

p {
  color: rgba(255,255,255,0.85);
}
</style>

<script>
setTimeout(function(){
  window.location.href = "/";
}, 2000);
</script>

</head>

<body>

<div class="card">
  <h2 class="success">QR Received</h2>
  <p>ID Stored Successfully</p>
  <p>Redirecting...</p>
</div>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

// 🔷 Fingerprint verification
void handleVerify() {
  int fingerID = getFingerprintID();
  String name = getNameFromID(fingerID);

  Serial.print("Fingerprint ID: ");
  Serial.println(fingerID);
  Serial.print("Person: ");
  Serial.println(name);

  String result;
  String status;

  if (fingerID == qrID) {
    result = "<span style='color:#22c55e;font-weight:bold;'>"
         + name + " Identified ✅</span>";
    status = "MATCH";
  } else {
    result = "<span style='color:#ef4444;font-weight:bold;'>Identity Mismatch ❌</span>";
    status = "MISMATCH";
  }

  sendToSheet(qrID, fingerID, status, name);
  server.send(200, "text/html", result);
}

// 🔷 Fingerprint function
int getFingerprintID() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

void setup() {
  Serial.begin(115200);

  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor ready");
  } else {
    Serial.println("Fingerprint sensor error");
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin("5G", "1234567890");
  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/send", handleQR);
  server.on("/verify", handleVerify);

  server.begin();
}

// 🔷 Send to Google Sheets
void sendToSheet(int qr, int fid, String status, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = scriptURL + 
                 "?qr=" + String(qr) + 
                 "&fid=" + String(fid) + 
                 "&name=" + name +
                 "&status=" + status;

    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();
    Serial.println("Sheet Response: " + String(httpCode));

    http.end();
  }
}

// Used for Name ID mapping
String getNameFromID(int id) {
  switch(id) {
    case 1: return "Vipul Pawar";
    case 2: return "Rahul Sharma";
    case 3: return "Amit Patil";
    case 4: return "Sneha Joshi";
    default: return "Unknown";
  }
}

void loop() {
  server.handleClient();
}
