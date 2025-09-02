#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>

const char* ssid = "heicantik";
const char* password = "iloveyou";
const char* serverName = "https://script.google.com/macros/s/AKfycbzZyqCUp6VIL-eRqcwlelHHZDvcNIJQ9KXqJqqehZYL4ETFYe5nkAruw4IqaDizT9lc/exec";

// UART2 setup for Uno → ESP32
HardwareSerial SerialPort(2);
#define RXD2 16
#define TXD2 17  // not used

void setup() {
  Serial.begin(115200);  // Debug monitor
  SerialPort.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Uno TX → GPIO16

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi");
}

void loop() {
  if (SerialPort.available()) {
    String line = SerialPort.readStringUntil('\n');
    line.trim();  // remove \r or space

    // Expected format: "6.72|Stable"
    int sep = line.indexOf('|');
    if (sep > 0) {
      String ph = line.substring(0, sep);
      String status = line.substring(sep + 1);

      Serial.println("Parsed → pH: " + ph + ", Status: " + status);

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        String jsonData = "{\"ph\": " + ph +
                          ", \"status\": \"" + status + "\"}";

        int httpCode = http.POST(jsonData);
        Serial.print("POST → ");
        Serial.println(httpCode);
        http.end();
      }
    } else {
      Serial.println("Invalid format: " + line);
    }
  }

  delay(100);  // light debounce
}
