#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

// === LCD Setup ===
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// === RTC Setup ===
RTC_DS1307 rtc;

// === Pins ===
const int redPin = 6;
const int greenPin = 7;
const int bluePin = 8;
const int buzzerPin = 9;
const int buttonPin = 10;
const int lightSensorPin = A0;

// === Pet State ===
bool asleep = false;
bool buzzerOn = false;
bool petDisappeared = false;
int revivalPressCount = 0;
unsigned long lastUpdate = 0;
int animationFrame = 0;
DateTime lastFedTime;

// === Light Sleep Handling ===
unsigned long darknessStartTime = 0;
const int darknessThreshold = 200;
const unsigned long sleepDelay = 10000;

// === Timing ===
int noteDuration = 200;

// === EEPROM Feed Time Storage ===
const int EEPROM_FEED_ADDR = 0;

// === Function Prototypes ===
void showFace(const char* face, const char* text, bool showTime = false);
void saveLastFedToEEPROM(DateTime t);
DateTime readLastFedFromEEPROM();

// === Tunes ===
int wakeTune[] = {262, 294, 330, 349, 392, 440, 494, 523, 440, 392};
int sleepTune[] = {523, 494, 440, 392, 349, 330, 294, 262, 196, 0};

void setup() {
  lcd.begin(16, 2);
  Wire.begin();
  rtc.begin();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.setCursor(0, 0);
  lcd.print("     Hello!");
  playTune(wakeTune, 10, noteDuration);
  delay(2000);
  lcd.clear();

  lastFedTime = readLastFedFromEEPROM();
  showFace("(^_^)", "", true);
}

void loop() {
  DateTime now = rtc.now();
  handleButtonPress(now);

  int lightLevel = analogRead(lightSensorPin);
  unsigned long currentMillis = millis();

  // Track darkness duration
  if (lightLevel < darknessThreshold) {
    if (darknessStartTime == 0) {
      darknessStartTime = currentMillis;
    }

    if (!asleep && (currentMillis - darknessStartTime >= sleepDelay)) {
      asleep = true;
      lcd.clear();
      showFace("(-_-)", "Goodnight!");
      playTune(sleepTune, 10, noteDuration);
      delay(2500);
      lcd.clear();
      showFace("(-_-)", "Zzz... Sleeping");
      setRGB(0, 0, 0);
      delay(2000);
    }
  } else {
    // Light is detected again
    darknessStartTime = 0;

    if (asleep) {
      asleep = false;
      lcd.clear();
      showFace("(0_0)", "Good Morning!!");
      playTune(wakeTune, 10, noteDuration);
      delay(2500);
      lcd.clear();
      showFace("(^_^)", "", true);
    }
  }

  // Run idle sleep animation only if asleep
  if (asleep) {
    idleSleep();
    return;
  }

  TimeSpan timeSinceFed = now - lastFedTime;

  if (!petDisappeared && timeSinceFed.totalseconds() >= 172800) {
    petDisappeared = true;
    lcd.clear();
    noTone(buzzerPin);
    buzzerOn = false;
  } else if (!petDisappeared && timeSinceFed.totalseconds() >= 86400 && !buzzerOn) {
    tone(buzzerPin, 800);
    buzzerOn = true;
  }

  if (petDisappeared && revivalPressCount >= 5) {
    petDisappeared = false;
    revivalPressCount = 0;
    lastFedTime = now;
    saveLastFedToEEPROM(now);
    lcd.clear();
    showFace("(T_T)", "You forgot me!");
    delay(3000);
    lcd.clear();
    showFace("(^_^)", "", true);
  }

  if (!petDisappeared && millis() - lastUpdate > 3000) {
    updateLCDAnimation();
    lastUpdate = millis();
  }

  if (!petDisappeared) {
    pulseRGB();
  } else {
    setRGB(0, 0, 0);
  }

  delay(30);
}

void handleButtonPress(DateTime now) {
  static bool lastState = HIGH;
  bool currentState = digitalRead(buttonPin);

  if (lastState == HIGH && currentState == LOW) {
    if (petDisappeared) {
      revivalPressCount++;
    } else {
      lastFedTime = now;
      saveLastFedToEEPROM(now);
      noTone(buzzerPin);
      buzzerOn = false;
      lcd.clear();
      showFace("(^_^)", "I'm so stuffed!!");
      playTone(600, 200);
      delay(1000);
      lcd.clear();
      showFace("(^_^)", "", true);
    }
  }
  lastState = currentState;
}

String formatTimeSinceLastFeed(TimeSpan span) {
  int totalHours = span.totalseconds() / 3600;
  int mins = (span.totalseconds() % 3600) / 60;

  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Fed: %dH %dM", totalHours, mins);
  return String(buffer);
}

void updateLCDAnimation() {
  const char* faces[] = {"(^_^)", "(o_o)", "(-_-)", "(^o^)"};
  lcd.clear();

  const char* face = faces[animationFrame % 4];
  TimeSpan span = rtc.now() - lastFedTime;
  String timeStr = formatTimeSinceLastFeed(span);

  lcd.setCursor(0, 0);
  lcd.print("     " + String(face));
  lcd.setCursor(0, 1);
  lcd.print(timeStr);

  animationFrame++;
}

void showFace(const char* face, const char* text, bool showTime) {
  lcd.setCursor(0, 0);
  lcd.print("     " + String(face));
  lcd.setCursor(0, 1);

  if (showTime) {
    TimeSpan span = rtc.now() - lastFedTime;
    String timeStr = formatTimeSinceLastFeed(span);
    lcd.print(timeStr);
  } else {
    lcd.print(text);
  }
}

void playTone(int freq, int duration) {
  if (freq > 0) tone(buzzerPin, freq, duration);
  delay(duration + 30);
  noTone(buzzerPin);
}

void playTune(int notes[], int length, int duration) {
  for (int i = 0; i < length; i++) {
    playTone(notes[i], duration);
  }
}

void setRGB(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void pulseRGB() {
  static int brightness = 0;
  static int fadeAmount = 5;

  setRGB(brightness, 0, 255 - brightness);
  brightness += fadeAmount;
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
}

void idleSleep() {
  setRGB(0, 0, 0);
  delay(5000);
}

void saveLastFedToEEPROM(DateTime t) {
  EEPROM.put(0, t.unixtime());
}

DateTime readLastFedFromEEPROM() {
  uint32_t unixTime = 0;
  EEPROM.get(0, unixTime);
  if (unixTime < 946684800UL || unixTime > 4102444800UL) {
    return rtc.now();
  }
  return DateTime(unixTime);
}
