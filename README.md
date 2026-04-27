## 🚧 Under Construction
-- Minor Project

# Initial Work Flow:

1. Mobile scans QR → gets ID (e.g., 3)  
2. Mobile sends "3" → ESP32  
3. ESP32 waits for fingerprint  
4. Fingerprint scanned → returns ID (e.g., 3)  
5. Compare:  
   if (QR_ID == FP_ID)  
       → SUCCESS  
   else  
       → REJECT  
6. Store result



Apps Script for data loging automation 

'''

function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();

  // 🔷 Get parameters
  var qr = e.parameter.qr || "N/A";
  var fid = e.parameter.fid || "N/A";
  var name = e.parameter.name || "Unknown";
  var status = e.parameter.status || "N/A";

  // 🔷 Add header row only once
  if (sheet.getLastRow() === 0) {
    sheet.appendRow(["QR_ID", "Finger_ID", "Name", "Status", "Timestamp"]);
  }

  // 🔷 Append data
  sheet.appendRow([qr, fid, name, status, new Date()]);

  return ContentService.createTextOutput("OK");
}

'''
