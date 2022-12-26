#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // add this library to your project
#include <GxEPD2_BW.h>
#include <Fonts/FreeSerifBold24pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include "time.h"

#include "wifi_epaper_settings.h"

#define ENABLE_GxEPD2_GFX 0

// Buffer to store the HTTP request
char request[1000];

// Buffer to store the HTTP response
char response[1000];

WiFiClientSecure client;
const String tibberApi = "https://api.tibber.com/v1-beta/gql";
const String payload = "{\"query\": \"{viewer {homes {currentSubscription {priceInfo {current {total}}}}}}\" }";

void setup() {
  // Initialize serial and WiFi
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi!");
  client.setCACert(root_ca);
  Serial.println("setup display");
  delay(100);
  display.init();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}

void loop() {
  HTTPClient http;
  String b = "Bearer ";

  http.begin(client, tibberApi); 
  // add necessary headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization",  b+token);
  int httpCode = http.POST(payload);
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    //Serial.println(response);
    double totalCostPerkwh = parseResponse(response);
    showCost(totalCostPerkwh);
  } else {
    Serial.println("something went wrong");
    Serial.println(httpCode);
  }
  http.end();

  // Disconnect from the server
  client.stop();

  int oneHourInMilliSeconds = 60*60*1000;
  // Wait a while before making another request
  delay(oneHourInMilliSeconds);
}

double parseResponse(String response) {

  // Parse the JSON response
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.println();
    Serial.println("Error parsing JSON response");
    return 0;
  }

  // Get the "total" value from the response
  double total = doc["data"]["viewer"]["homes"][0]["currentSubscription"]["priceInfo"]["current"]["total"];

  // Print the total to the serial console
  Serial.println(total);
  return total;
}

void showCost(double dCost) {
  Serial.println("showCost");
  display.setRotation(1);
  display.setFont(&FreeSerifBold24pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  String sCost = " kr";
  String dispText = dCost + sCost;
  display.getTextBounds(dispText, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(dispText);
    display.setFont(&FreeSerifBold9pt7b);
    display.setCursor(x + tbw + 2, y);
    display.print("/kWh");
    
    showLocalTime(x+2,20);
  }
  while (display.nextPage());
  Serial.println("showCost done");
}

void showLocalTime(int16_t x, int16_t y) {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

  char timeNow[20];
  strftime(timeNow,20, "%Y-%m-%d %H:%M:%S", &timeinfo);
  display.setFont(&FreeSerifBold9pt7b);
  display.setCursor(x,y);
  display.print(timeNow);
}
