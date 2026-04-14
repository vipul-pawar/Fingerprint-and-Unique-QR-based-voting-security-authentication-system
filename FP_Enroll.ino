#include <Adafruit_Fingerprint.h>

// Use UART2 of ESP32
HardwareSerial mySerial(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nESP32 Fingerprint Enrollment");

  // Initialize UART2 (RX=16, TX=17)
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("✅ Found fingerprint sensor!");
  } else {
    Serial.println("❌ Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  // Display sensor details (optional but useful)
  Serial.println("Reading sensor parameters...");
  finger.getParameters();
  Serial.print("Capacity: "); Serial.println(finger.capacity);
  Serial.print("Security level: "); Serial.println(finger.security_level);
  Serial.print("Packet length: "); Serial.println(finger.packet_len);
  Serial.print("Baud rate: "); Serial.println(finger.baud_rate);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  Serial.println("\nReady to enroll a fingerprint!");
  Serial.println("Enter ID (1–127): ");

  id = readnumber();

  if (id == 0) {
    Serial.println("Invalid ID");
    return;
  }

  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll());
}

uint8_t getFingerprintEnroll() {

  int p = -1;

  Serial.println("👉 Place finger...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("\nCommunication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("\nImaging error");
        break;
      default:
        Serial.println("\nUnknown error");
        break;
    }
  }

  // Convert image
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image");
    return p;
  }

  Serial.println("👉 Remove finger...");
  delay(2000);

  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("👉 Place SAME finger again...");
  p = -1;

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Second image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      default:
        Serial.println("\nError");
        break;
    }
  }

  // Convert second image
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting second image");
    return p;
  }

  // Create model
  Serial.println("Creating fingerprint model...");
  p = finger.createModel();

  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else {
    Serial.println("Fingerprints did not match");
    return p;
  }

  // Store model
  Serial.print("Storing in ID ");
  Serial.println(id);

  p = finger.storeModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprint stored successfully!");
  } else {
    Serial.println("❌ Error storing fingerprint");
    return p;
  }

  return true;
}