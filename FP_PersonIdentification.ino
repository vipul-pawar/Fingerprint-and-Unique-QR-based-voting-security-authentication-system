#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// 🔷 WiFi credentials
const char* ssid = "ESP32_QR_SYSTEM";
const char* password = "12345678";

WebServer server(80);

// 🔷 Fingerprint setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// 🔷 Store QR ID
int qrID = -1;

// 🔷 Homepage
void handleRoot() {
  String page = "<html><body>";
  page += "<h2>Identity Verification System</h2>";
  page += "<p>Step 1: Scan QR Code</p>";

  if (qrID != -1) {
    page += "<p>QR ID Received: " + String(qrID) + "</p>";
    page += "<p>Step 2: Place Finger</p>";
    page += "<button onclick=\"location.href='/verify'\">Verify Fingerprint</button>";
  }

  page += "</body></html>";

  server.send(200, "text/html", page);
}

// 🔷 QR handler
void handleQR() {
  String qrData = server.arg("qr");

  Serial.print("QR Received: ");
  Serial.println(qrData);

  // 🔷 Convert QR string to integer ID
  qrID = qrData.toInt();   // Example: "1" → 1

  server.send(200, "text/html", "<h3>QR Received. Now place finger.</h3><a href='/'>Go Back</a>");
}

// 🔷 Fingerprint verification
void handleVerify() {
  int fingerID = getFingerprintID();

  Serial.print("Fingerprint ID: ");
  Serial.println(fingerID);

  String result;

  if (fingerID == -1) {
    result = "<h2 style='color:orange;'>No Finger Detected</h2>";
  }
  else if (fingerID == qrID) {
    result = "<h2 style='color:green;'>Person Identified ✅</h2>";
  }
  else {
    result = "<h2 style='color:red;'>Identity Mismatch ❌</h2>";
  }

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

  return finger.fingerID;  // returns stored ID
}

void setup() {
  Serial.begin(115200);

  // 🔷 Fingerprint init
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor ready");
  } else {
    Serial.println("Fingerprint sensor error");
  }

  // 🔷 WiFi AP
  WiFi.softAP(ssid, password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // 🔷 Routes
  server.on("/", handleRoot);
  server.on("/send", handleQR);
  server.on("/verify", handleVerify);

  server.begin();
}

void loop() {
  server.handleClient();
}