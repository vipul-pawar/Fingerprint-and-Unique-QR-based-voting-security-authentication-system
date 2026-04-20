## UnderConstruction 🚧
--Minor Project


# Initial Work Flow:-

"1. Mobile scans QR → gets ID (e.g., 3)
2. Mobile sends "3" → ESP32
3. ESP32 waits for fingerprint
4. Fingerprint scanned → returns ID (e.g., 3)
5. Compare:
     if (QR_ID == FP_ID)
         → SUCCESS
     else
         → REJECT
6. Store result
"
