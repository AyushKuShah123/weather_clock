#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Replace with your network credentials
const char* ssid = "123123123";
const char* password = "0987654321";

#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>  // Include the LCD library

#define DHTPIN D4        // DHT11 connected to pin D4
#define DHTTYPE DHT11    // DHT 11 type sensor

RTC_DS3231 rtc;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the I2C address and size of LCD

#define ALARM_LED_PIN D0 // LED for alarm status
#define BUZZER_PIN 1     // Buzzer pin
#define MODE_SWITCH D8
#define MODE_SWITCH_PIN 3
#define INC_SWITCH_PIN D7
#define DEC_SWITCH_PIN D6
#define ALARM_SWITCH_PIN D5

bool alarmEnabled = true;   // Initialize alarm as disabled
bool alarmTriggered = true; // Track whether the alarm is triggered
bool alarmAM = true;         // Default alarm time format to AM
bool settingAlarm = true;   // Flag to track if we are setting the alarm

int alarmHour = 7;   // Default alarm hour set to 7
int alarmMinute = 0; // Default alarm minute
bool alarmModeAM = true; // Default alarm mode to AM (7 AM)

// Additional declarations
bool lastModeSwitchState = HIGH; // Keep track of the previous state for debounce
bool isModeSwitchPressed = false; // Flag to prevent multiple toggles


// NTP Server and timezone
const char* ntpServer = "pool.ntp.org"; // NTP server
const long utcOffsetInSeconds = 19800; // UTC +5:30 for India

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);
//RTC_DS1307 rtc; // Use the appropriate RTC library


void setup() {
  Serial.begin(115200);
  pinMode(ALARM_SWITCH_PIN, INPUT_PULLUP);  // Configure alarm switch with internal pull-up
  pinMode(INC_SWITCH_PIN, INPUT_PULLUP);    // Increment switch
  pinMode(DEC_SWITCH_PIN, INPUT_PULLUP);    // Decrement switch
  pinMode(ALARM_LED_PIN, OUTPUT);           // Alarm LED
  pinMode(BUZZER_PIN, OUTPUT);              // Buzzer pin
  digitalWrite(ALARM_LED_PIN, LOW);         // Ensure LED is off at startup
  digitalWrite(BUZZER_PIN, LOW);            // Ensure buzzer is off at startup
  dht.begin();

    // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
    if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // OLED address
  lcd.init();  // Initialize the LCD
  lcd.backlight();  // Turn on the backlight

  display.clearDisplay();
  display.display();
  


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi ... ...");
    loop();
  }
  Serial.println("Connected to WiFi");

  // Start the NTP client
  timeClient.begin();
  timeClient.update();

  // Print current time
  Serial.println("Current time:");
  Serial.print("Unix Timestamp: ");
  Serial.println(timeClient.getEpochTime());

  // Update RTC with current time
  DateTime now = DateTime(timeClient.getEpochTime());
  rtc.adjust(now);
  Serial.print("RTC updated: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}


void loop() {
  DateTime now = rtc.now();
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Handle alarm switch press
  if (digitalRead(ALARM_SWITCH_PIN) == LOW) {
    delay(200); // Simple debounce
    if (settingAlarm) {
      settingAlarm = false; // Toggle off setting mode
    } else {
      alarmEnabled = !alarmEnabled; // Toggle alarm on/off
      settingAlarm = true; // Enter alarm setting mode when alarm is enabled
    }
  }

  // Continuously update the LED to show the alarm status
  digitalWrite(ALARM_LED_PIN, alarmEnabled ? HIGH : LOW); // LED reflects alarm state

  // Handle alarm setting (when in settingAlarm mode)
  if (settingAlarm) {
    if (digitalRead(INC_SWITCH_PIN) == LOW) {
      delay(200); // Simple debounce
      alarmHour++;
      if (alarmHour > 12) {
        alarmHour = 1; // Reset to 1 after 12
        alarmModeAM = !alarmModeAM; // Toggle AM/PM
      }
    }
    if (digitalRead(DEC_SWITCH_PIN) == LOW) {
      delay(200); // Simple debounce
      alarmMinute += 5;
      if (alarmMinute >= 60) alarmMinute = 0;
    }
  }

  // Display time, temperature, humidity, and alarm status on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Show date and time
  display.setCursor(0, 0);
  display.print(" ");
   display.print(now.day() < 10 ? '0' : ' ');
  display.print(now.day(), DEC);
  display.print('/');
  display.print(now.month() < 10 ? '0' : ' ');
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.year(), DEC);

  // Show time in 12-hour format
  int displayHour = now.hour() % 12;
  if (displayHour == 0) displayHour = 12; // 12-hour format adjustment

  display.setCursor(0, 10);
  display.setTextSize(2); // Increase text size for time
  display.print(displayHour < 10 ? '0' : ' '); // Add leading zero if necessary
  display.print(displayHour, DEC);
  display.print(':');
  display.print(now.minute() < 10 ? '0' : ' '); // Add leading zero for minute
  display.print(now.minute(), DEC);
  display.print(':');
  display.print(now.second());

  // Show temperature and humidity
  display.setTextSize(1); // Reset text size to normal for other text
  display.setCursor(0, 30);
  display.print("Temp: ");
  display.print(temp);
  display.print(" ");
  display.print((char)223);
  display.print(" C  ");
  
  display.print(now.hour() >= 12 ? " PM" : " AM"); // Display AM/PM

  display.setCursor(0, 40);
  display.print("Humidity: ");
  display.print(humidity);
  display.print(" %");

  // Show alarm status
  display.setCursor(0, 50);
  if (alarmEnabled) {
    display.print("Alarm : ON ");
    
    display.print(alarmHour);
    display.print(':');
    display.print(alarmMinute < 10 ? '0' : ' ');
    display.print(alarmMinute);
    
    display.print(alarmModeAM ? " AM" : " PM");
  } else {
    display.print("Alarm : OFF");
  }

  display.display(); // Update OLED

  // Display time, temperature, and alarm status on LCD
  lcd.clear(); // Clear the LCD display
  lcd.setCursor(0, 0);
  lcd.print(" ");
  lcd.print(displayHour < 10 ? '0' : ' '); // Add leading zero
  lcd.print(displayHour);
  lcd.print(':');
  lcd.print(now.minute() < 10 ? '0' : ' '); // Add leading zero
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second() < 10 ? '0' : ' '); // Add leading zero
  lcd.print(now.second());
  lcd.print(now.hour() >= 12 ? " PM" : " AM"); // Display AM/PM
  
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.print(temp,1);
  lcd.print((char)223);
  lcd.print("C ");
  
  lcd.print(alarmEnabled ? "Alrm:ON" : "Alrm:OFF");

  // Alarm logic: Check if current time matches the alarm time
  int alarmAdjustedHour = alarmModeAM ? alarmHour : alarmHour + 12; // Handle AM/PM adjustment
  if (alarmEnabled && now.hour() == alarmAdjustedHour && now.minute() == alarmMinute && !alarmTriggered) {
    triggerAlarm();
  } else if (now.minute() != alarmMinute || now.hour() != alarmAdjustedHour) {
    alarmTriggered = false;  // Reset alarm trigger after the alarm time has passed
  }

  delay(500); // Update every second
}

void triggerAlarm() {
  alarmTriggered = true;
  digitalWrite(BUZZER_PIN, HIGH); // Sound the alarm
  for (int i = 0; i < 5; i++) {
    delay(1000);                   // Alarm duration (1 seconds)
    digitalWrite(BUZZER_PIN, LOW);  // Turn off the alarm
    delay(500);
    digitalWrite(BUZZER_PIN, HIGH); // Sound the alarm
  }
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the alarm
}