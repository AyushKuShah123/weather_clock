

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "shivamgifthouse_fbdgt";
const char* password = "lNTEIIlGENCE";

// OpenWeatherMap API details
const char* apiKey = "cf845e26e7de05cfe71945c32c0cbe2e";
const char* city = "Butwal";
const char* countryCode = "np";

// Time client settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 20700, 60000); // Nepal timezone offset is +5:45 hours (20700 seconds)

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Wire.begin(14, 15); // Initialize I2C communication with SDA on GPIO 14 and SCL on GPIO 15
  pinMode(33, OUTPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    
    display.setTextSize(1); 
    Serial.println("Connecting to WiFi...");
    display.println("  WIFI_CONNECTing ");
    
    digitalWrite(33, HIGH);
    delay(300);
    digitalWrite(33, LOW);
    delay(300);
    
  }
  Serial.println("Connected to WiFi");
  
  
  display.setTextSize(2); 
  display.println("  WIFI_CONNECTED ");
  
  delay(300);
  digitalWrite(33, LOW);

  // Initialize NTPClient to get time
  timeClient.begin();
}

void loop() {
  digitalWrite(33, LOW);
  // Get current time
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  String formattedDate = getDate();

  // Get current weather
  String weather = getWeather();

  // Display time and weather on OLED
  display.clearDisplay();
  display.setTextSize(2); // Increased text size
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
   display.print(" ");
  //display.print("Time:");
  display.println(formattedTime);

  //display.setCursor(0, 18);
  //display.print("Date:");
  display.println(formattedDate);

  //display.setCursor(0, 36);
  //display.print("Weather: ");
  display.println(weather);

  display.display();

  delay(2400); // Update every 30 seconds
}

String getWeather() {
  String weather = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(countryCode) + "&appid=" + String(apiKey) + "&units=metric";

    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);

      float temperature = doc["main"]["temp"];
      const char* weatherDescription = doc["weather"][0]["description"];
      weather = +" " + String(temperature) +" "+ (char)247 +"C" +" "+ String(weatherDescription);
    } else {
      weather = "Error: " + String(httpResponseCode);
    }
    http.end();
  } else {
    weather = "WiFi Disconnected";
        delay(300);
    digitalWrite(33, HIGH);
    delay(300);
  }
  return weather;
}

String getDate() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  char buffer[11];
  strftime(buffer, 11, "%d/%m/%Y", timeInfo);
  return String(buffer);
}
