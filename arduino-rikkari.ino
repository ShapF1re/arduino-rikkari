const int rs = 12;
const int en = 11;
const int d4 = 5;
const int d5 = 4;
const int d6 = 3;
const int d7 = 2;


const int trigPin = 8;
const int echoPin = 9;
const int speakerPin = 10;
const int buttonPin = 7;
const int redLedPin = 13;
const int greenLedPin = 6;


float duration, distance;
int alertCount = 0;
bool monitoring = false;
bool sirenState = false;
unsigned long monitoringStartTime = 0;
bool delayActive = false;
unsigned long intrusionTime = 0;
bool intrusionDetected = false;
bool sirenRunning = false;
unsigned long sirenStartTime = 0;
int buttonPressCount = 0;
unsigned long firstPressTime = 0;
const unsigned long pressWindow = 5000;
bool buttonLastState = HIGH;
unsigned long sensorFailStartTime = 0;
bool sensorFailDetected = false;


String history[10];
int historyIndex = 0;


unsigned long cooldownStartTime = 0;
const unsigned long cooldownTime = 10000;


LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  Serial.begin(9600);
  noTone(speakerPin);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Alarm OFF");
}


void loop() {
  bool buttonState = digitalRead(buttonPin) == LOW;


  if (buttonState && !buttonLastState) {
    if (!monitoring) {
      delayActive = true;
      monitoringStartTime = millis();
      lcd.clear();
      lcd.print("Starting...");
    } else {
      monitoring = false;
      lcd.clear();
      lcd.print("Alarm OFF");
      noTone(speakerPin);
      sirenState = false;
      sirenRunning = false;
      digitalWrite(redLedPin, LOW);
      digitalWrite(greenLedPin, HIGH);
    }
  }


  buttonLastState = buttonState;


  if (millis() - firstPressTime > pressWindow) {
    buttonPressCount = 0;
  }


  if (delayActive && millis() - monitoringStartTime >= 10000) {
    monitoring = true;
    delayActive = false;
    lcd.clear();
    lcd.print("Alarm ON");
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
  }


  if (!monitoring) return;


  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);


  duration = pulseIn(echoPin, HIGH, 30000);
 
  if (duration == 0 || duration > 30000) {
    if (!sensorFailDetected) {
      sensorFailDetected = true;
      sensorFailStartTime = millis();
    } else if (millis() - sensorFailStartTime >= 5000) {
      Serial.println("Sensor failure detected! Triggering alarm.");
      tone(speakerPin, 1000);
      lcd.clear();
      lcd.print("SENSOR ERROR");
      digitalWrite(redLedPin, HIGH);
      digitalWrite(greenLedPin, LOW);
    }
    return;
  }


  sensorFailDetected = false;


  distance = (duration * 0.0343) / 2;
  Serial.print("Distance: ");
  Serial.println(distance);


  if (distance < 50 && !sirenRunning) {
    if (!intrusionDetected) {
      intrusionDetected = true;
      intrusionTime = millis();
      lcd.clear();
      lcd.print("Threat Detected");
    } else if (millis() - intrusionTime >= 5000) {
      Serial.println("Siren ON!");
      tone(speakerPin, 1000);
      sirenState = true;
      sirenRunning = true;
      sirenStartTime = millis();
      alertCount++;
      lcd.clear();
      lcd.print("ALARM ACTIVE!");
      history[historyIndex] = "Threat detected at " + String(millis() / 1000) + "s";
      historyIndex = (historyIndex + 1) % 10;
    }
  }


  if (sirenRunning && millis() - sirenStartTime >= 30000) {
    Serial.println("Siren OFF (Timeout)!");
    noTone(speakerPin);
    sirenState = false;
    sirenRunning = false;
    intrusionDetected = false;
    lcd.clear();
    lcd.print("Alarm ON");
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }


  if (buttonState && sirenState && millis() - intrusionTime >= 5000) {
    if (millis() - cooldownStartTime >= cooldownTime) {
      Serial.println("Alarm Canceled");
      noTone(speakerPin);
      sirenState = false;
      sirenRunning = false;
      intrusionDetected = false;
      lcd.clear();
      lcd.print("Alarm ON");
      digitalWrite(redLedPin, LOW);
      digitalWrite(greenLedPin, HIGH);
      cooldownStartTime = millis();
    } else {
      lcd.clear();
      lcd.print("Cooldown active");
    }
    delay(300);
  }


  if (sensorFailDetected && buttonState) {
    lcd.clear();
    lcd.print("Resetting sensor...");
    delay(1000);


    duration = pulseIn(echoPin, HIGH, 30000);
    if (duration != 0 && duration < 30000) {
      sensorFailDetected = false;
      lcd.clear();
      lcd.print("Sensor OK");
      digitalWrite(redLedPin, LOW);
      digitalWrite(greenLedPin, HIGH);
    } else {
      lcd.clear();
      lcd.print("SENSOR ERROR");
    }
  }


  lcd.setCursor(0, 1);
  lcd.print("Alerts: ");
  lcd.print(alertCount);


  delay(500);
}



