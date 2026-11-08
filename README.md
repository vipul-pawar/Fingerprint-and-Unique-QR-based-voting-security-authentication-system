# Fingerprint and Unique QR Based Voting Security Authentication System

An ESP32-based voter authentication system that combines a **unique QR code** (something the voter has) with **fingerprint verification** (something the voter is). A voter is authenticated only when the fingerprint scanned matches the ID encoded in their QR code. Every attempt is logged to **Google Sheets** for an audit trail.

> Repository name: `Fingerprint-and-Unique-QR-based-voting-security-authentication-system`

---

## Problem Statement

Traditional voting is exposed to impersonation, proxy voting and fake voter IDs. A single factor, such as a card or a paper slip, can be stolen, copied or shared. This project adds a second, biometric factor so that holding someone else's QR code is not enough to be authenticated as that voter.

## Features

- Unique QR code per voter, carrying the voter ID
- Fingerprint verification using an Adafruit-compatible sensor (R307 / AS608 type)
- Two-factor check: QR ID must equal the fingerprint ID
- Displays the voter's name on a successful match, and a clear mismatch message otherwise
- Runs its own Wi-Fi hotspot, so no router is needed at the polling desk
- Mobile-friendly web interface
- Automatic data logging of QR ID, fingerprint ID, name, status (MATCH / MISMATCH) and timestamp to Google Sheets
- Works in `WIFI_AP_STA` mode: hotspot for the voter, station mode for internet access

---

## How It Works

```
┌────────────┐   1. Scan QR (opens /send?qr=ID)   ┌──────────────┐
│   Phone    │ ─────────────────────────────────► │              │
│  (browser) │                                    │    ESP32     │ ◄── Fingerprint
│            │ ◄───────────────────────────────── │  (Web Server)│     Sensor
└────────────┘   2. Tap "Verify Fingerprint"      └──────┬───────┘
                    3. Result shown on page              │
                                                         │ 4. HTTP GET
                                                         ▼
                                                 Google Apps Script
                                                         │
                                                         ▼
                                                   Google Sheets
                                                  (verification log)
```

1. The voter connects to the ESP32 hotspot.
2. The voter scans their unique QR code, which opens `http://192.168.4.1/send?qr=<VOTER_ID>`. The ESP32 stores the ID and redirects to the home page.
3. The voter taps **Verify Fingerprint** and places a finger on the sensor.
4. The ESP32 compares the matched fingerprint ID with the stored QR ID.
5. The result (**Identified ✅** or **Identity Mismatch ❌**) is shown and logged to Google Sheets.

---

## Hardware Required

| Component | Notes |
|-----------|-------|
| ESP32 development board | Any ESP32 DevKit |
| Fingerprint sensor | R307 / AS608 (Adafruit Fingerprint library compatible) |
| Jumper wires | |
| USB cable | For power and programming |
| Phone | To scan QR codes and open the web page |

### Wiring

| Fingerprint Sensor | ESP32 |
|--------------------|-------|
| VCC | 3.3V (check your sensor's rated voltage) |
| GND | GND |
| TX | GPIO 16 (RX2) |
| RX | GPIO 17 (TX2) |

The sensor communicates over `Serial2` at **57600 baud**.

---

## Software and Libraries

- [Arduino IDE](https://www.arduino.cc/en/software) with the **ESP32 board package** installed
- Libraries:
  - `WiFi`, `WebServer`, `HTTPClient`, `HardwareSerial` (built in with the ESP32 core)
  - [`Adafruit Fingerprint Sensor Library`](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library) (install from Library Manager)

---

## Setup

### 1. Enroll voter fingerprints

Enroll each voter's fingerprint using the **Enroll** example from the Adafruit Fingerprint library. The ID you assign to each finger **must match** the voter's QR ID and the name mapping in the code.

### 2. Set up Google Sheets logging

1. Create a new Google Sheet. You don't need to add headers manually, because the script creates them on the first entry: `QR_ID | Finger_ID | Name | Status | Timestamp`
2. Open **Extensions → Apps Script** and paste:

   ```javascript
   function doGet(e) {
     var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();

     // Get parameters
     var qr = e.parameter.qr || "N/A";
     var fid = e.parameter.fid || "N/A";
     var name = e.parameter.name || "Unknown";
     var status = e.parameter.status || "N/A";

     // Add header row only once
     if (sheet.getLastRow() === 0) {
       sheet.appendRow(["QR_ID", "Finger_ID", "Name", "Status", "Timestamp"]);
     }

     // Append data
     sheet.appendRow([qr, fid, name, status, new Date()]);

     return ContentService.createTextOutput("OK");
   }
   ```

3. Click **Deploy → New deployment → Web app**
   - Execute as: **Me**
   - Who has access: **Anyone**
4. Copy the **Web app URL** and paste it into `scriptURL` in the code.

Each verification attempt is logged automatically as a new row with its timestamp. If you edit the script later, create a **new deployment version** so the changes take effect.

### 3. Configure the code

```cpp
// Hotspot created by the ESP32
const char* ssid     = "ESP32_QR_SYSTEM";
const char* password = "12345678";

// Google Apps Script web app URL
String scriptURL = "YOUR_GOOGLE_SCRIPT_URL";

// Internet Wi-Fi the ESP32 connects to (inside setup())
WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
```

Map voter IDs to names:

```cpp
String getNameFromID(int id) {
  switch(id) {
    case 1: return "Vipul Pawar";
    case 2: return "Rahul Sharma";
    case 3: return "Amit Patil";
    case 4: return "Sneha Joshi";
    default: return "Unknown";
  }
}
```

### 4. Upload

1. Select your ESP32 board and COM port in the Arduino IDE
2. Upload the sketch
3. Open the Serial Monitor at **115200 baud**. You should see `Fingerprint sensor ready` and the hotspot IP (`192.168.4.1`).

### 5. Generate unique QR codes

Create one QR code per voter (any QR generator works) encoding:

```
http://192.168.4.1/send?qr=1
```

Replace `1` with each voter's unique ID (`2`, `3`, `4`, ...).

---

## Usage

1. Connect your phone to the Wi-Fi network **ESP32_QR_SYSTEM** (password: `12345678`).
2. Scan the voter's QR code. The page shows **QR Received** and redirects to the home screen.
3. Confirm the **QR ID** is shown on the home page.
4. Tap **Verify Fingerprint** and place the finger on the sensor.
5. **Match** means the voter is authenticated. **Mismatch** means authentication failed. Both are logged in Google Sheets.

---

## API Endpoints

| Endpoint | Description |
|----------|-------------|
| `/` | Home page showing the stored QR ID and the verify button |
| `/send?qr=<ID>` | Stores the scanned QR ID and redirects to `/` |
| `/verify` | Scans the fingerprint, compares it with the QR ID and returns the result |

---

## Project Structure

```
├── FP_Enroll.ino                # Enrolls fingerprints and assigns IDs on the sensor
├── FP_Matching.ino              # Tests fingerprint matching
├── FP_PersonIdentification.ino  # Main sketch: QR + fingerprint verification, web server, Google Sheets logging
├── screenshots/                 # Images used in this README
└── README.md
```

---

## Screenshots

### Full System (Hardware Setup)

![Full System](screenshots/Full%20System.jpg)

### Web Interface

![Web Interface](screenshots/web%20interface.jpg)

### Google Sheets Log

![Google Sheets Log](screenshots/Couds%20Logs.jpg)
---

## Scope and Limitations

This project handles the **authentication** stage of voting only. It does not record or count votes.

- No duplicate-vote prevention yet: a voter who is verified twice is simply logged twice
- Names and IDs are hard-coded; moving them to flash storage (SPIFFS/LittleFS) or a database would be cleaner
- The `name` parameter is not URL-encoded when sent to Google Sheets, so names with spaces may not be sent reliably
- The stored QR ID is not cleared after a verification attempt
- The QR code is a plain URL and could be copied, so security depends on the fingerprint check
- No HTTPS or admin authentication on the ESP32 web server

## Future Scope

- Mark each voter as "voted" and block repeat verification
- Integrate with an electronic voting machine or relay-based unlock
- Add an OLED/LCD display and buzzer for on-device feedback
- Encrypted or signed QR codes
- Admin dashboard with live turnout statistics
- Camera-based QR scanning with an ESP32-CAM

---

## Security Note

Do **not** commit your real Wi-Fi password or Google Apps Script URL to a public repository. Use placeholders before pushing to GitHub.

---

## Author

**Your Name**
Minor Project, Your College / Department, Year

---

## License

This project is released under the [MIT License](LICENSE).
