#include <Adafruit_Fingerprint.h>

// Use UART2
HardwareSerial mySerial(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nESP32 Fingerprint Matching");

  // RX = 16, TX = 17
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("✅ Sensor found!");
  } else {
    Serial.println("❌ Sensor NOT found");
    while (1);
  }
}

void loop() {
  getFingerprintID();
  delay(1000);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK) {
    Serial.println("No finger detected");
    return p;
  }

  Serial.println("Image taken");

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image");
    return p;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Match found!");
    Serial.print("ID: ");
    Serial.println(finger.fingerID);
    Serial.print("Confidence: ");
    Serial.println(finger.confidence);
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("❌ No match found");
  } else {
    Serial.println("Error searching");
  }

  return p;
}