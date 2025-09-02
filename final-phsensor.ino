#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int relayBase = 7;
const int relayAcid = 8;
const int pHpin = A0;

LiquidCrystal_I2C lcd(0x27, 20, 4);  // 20x4 LCD

// Thresholds
const float pH_low = 6;
const float pH_high = 8;
uint32_t elapsedLoop;

// Timing
const unsigned long pumpDuration = 5000;       // 5 seconds pumping
const unsigned long cooldownDuration = 60000;  // 60 seconds mixing
const unsigned long loopInterval = 1000;
// System state
enum SystemState { IDLE,
                   INJECTING,
                   COOLING };
SystemState currentState = IDLE;

unsigned long stateStartTime = 0;
String currentStatus = "Stable";
String currentPump = "None";
String currentPhase = "IDLE";
String pumpLabel;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(relayBase, OUTPUT);
  pinMode(relayAcid, OUTPUT);
  digitalWrite(relayBase, HIGH);  // OFF
  digitalWrite(relayAcid, HIGH);  // OFF

  lcd.setCursor(0, 0);
  lcd.setCursor(0, 0);
  lcd.print("Smart pH System");
  lcd.setCursor(0, 1);
  lcd.print("Booted");
  delay(2000);
  lcd.clear();
}
float pH;
void loop() {
  unsigned long now = millis();
  if (now - elapsedLoop >= loopInterval) {
    elapsedLoop = now;
    pH = readPH();
    displayStatus(pH);
    Serial.print(pH, 2);
    Serial.print("|");
    Serial.println(currentStatus);
    if (pH < pH_low) {
      currentStatus = "Too Acid";
    } else if (pH > pH_high) {
      currentStatus = "Too Base";
    } else {
      currentPump = "None";
      currentStatus = "Stable";
      //displayStatus(pH);
    }
  }
  switch (currentState) {
    case IDLE:
      currentPhase = "IDLE";
      if (pH < pH_low) {
        // currentStatus = "Too Acid";
        startInjection(relayBase, "Base", "Too Acid");
      } else if (pH > pH_high) {
        // currentStatus = "Too Base";
        startInjection(relayAcid, "Acid", "Too Base");
      } else {
        currentPump = "None";
        // currentStatus = "Stable";
        //displayStatus(pH);
      }
      break;

    case INJECTING:
      currentPhase = "INJECTING";
      if (now - stateStartTime >= pumpDuration) {
        digitalWrite(relayBase, HIGH);
        digitalWrite(relayAcid, HIGH);
        stateStartTime = now;
        currentState = COOLING;
        currentPhase = "COOLING DOWN";
        currentPump = "None";
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("Injection Done.");
        // lcd.setCursor(0, 1);
        // lcd.print("Waiting to mix...");
        // lcd.setCursor(0, 2);
        // lcd.print("Cooldown: 20 sec");
        // lcd.setCursor(0, 3);
        // lcd.print("System: COOLING");
      }
      break;

      static unsigned long lastCountdownUpdate = 0;

    case COOLING:
      unsigned long elapsed = now - stateStartTime;
      if (elapsed >= cooldownDuration) {
        currentState = IDLE;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cooldown Complete");
        lcd.setCursor(0, 1);
        lcd.print("Resuming check...");
        delay(2000);
      } else {
        // Only update LCD every 1 second
        if (now - lastCountdownUpdate >= 1000) {
          int remainingSec = (cooldownDuration - elapsed) / 1000;
          lastCountdownUpdate = now;

          // lcd.clear();
          // lcd.setCursor(0, 0);
          // lcd.print("Injection Done.");
          // lcd.setCursor(0, 1);
          // lcd.print("Waiting to mix...");
          // lcd.setCursor(0, 2);
          // lcd.print("Cooldown: ");
          // lcd.print(remainingSec);
          // lcd.print(" sec");
          // lcd.setCursor(0, 3);
          // lcd.print("System: COOLING");

          Serial.print("Cooldown remaining: ");
          Serial.print(remainingSec);
          Serial.println(" sec");
        }
      }
      break;
  }
}

// === FUNCTIONS ===

float readPH() {
  int raw = analogRead(pHpin);
  float voltage = raw * (5.0 / 1023.0);
  return 14.58 * voltage - 21.31;
}

void startInjection(int relayPin, String pumpLabel, String status) {
  digitalWrite(relayBase, HIGH);
  digitalWrite(relayAcid, HIGH);
  digitalWrite(relayPin, LOW);  // ON selected pump

  stateStartTime = millis();
  currentState = INJECTING;
  currentStatus = status;
  currentPump = pumpLabel;

  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("pH: ");
  // lcd.print(readPH(), 2);
  // lcd.setCursor(0, 1);
  // lcd.print("Status: " + status);
  // lcd.setCursor(0, 2);
  // lcd.print("Action: " + pumpLabel);
  // lcd.setCursor(0, 3);
  // lcd.print("System: INJECTING");
}

void displayStatus(float pH) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH: ");
  lcd.print(pH, 2);
  lcd.setCursor(0, 1);
  lcd.print("Status: " + currentStatus);
  lcd.setCursor(0, 2);
  lcd.print("Pump: " + currentPump);
  lcd.setCursor(0, 3);
  lcd.print("System: " + currentPhase);
}