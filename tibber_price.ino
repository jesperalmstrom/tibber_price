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

StaticJsonDocument<200> parseToJsonDoc(String response) {
  // Parse the JSON response
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.println();
    Serial.println("Error parsing JSON response");
    //return null;
  }
  return doc;
}

void showCost(double dCost) {
  Serial.println("showCost");
  display.setRotation(3);
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
  do {
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

void showConsumtion(double dCost, double dConsumtion) {
  Serial.println("showConsumtion");
  display.setRotation(3);
  display.setFont(&FreeSerifBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  String sCost = " kr";
  String dispText = dCost + sCost;
  String dispTextConsumtion = dConsumtion + sCost;
  display.getTextBounds(dispText, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 4) - tbx;
  uint16_t y = ((display.height() + tbh) / 2) + tby;
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(dispText);
    display.setCursor(x + tbw + 5, y);
    display.print(dispTextConsumtion);
  }
  while (display.nextPage());
  Serial.println("showConsumtion done");
}

void showProduction(double dTotalProduction, double dTotalProfit) {
  Serial.println("showProduction");
  display.setRotation(3);
  display.setFont(&FreeSerifBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  String sKWH = " kWh";
  String dispText = dTotalProduction + sKWH;
  String sCurrent = " kr";
  String dispTextProfit = dTotalProfit + sCurrent;
  display.getTextBounds(dispText, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() + tbw) / 2) + tbx;
  uint16_t y = ((display.height() + tbh) / 2) + tby;
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(dispText);
    display.setCursor(x + tbw + 5, y);
    display.print(dispTextProfit);
  }
  while (display.nextPage());
  Serial.println("showProduction done");
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

char* getHoursOfDay() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "NA";
  }
  char hoursToday[2];
  strftime(hoursToday,2, "%H", &timeinfo);
  Serial.print("Hour of day ");
  Serial.println(hoursToday);
  return hoursToday;
}

void loop() {
  HTTPClient http;
  String b = "Bearer ";

  http.begin(client, tibberApi); 
  // add necessary headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization",  b+token);
  char* hours = getHoursOfDay();
  Serial.println(hours);
  String payload = "{\"query\": \"{viewer {homes {consumption(resolution: HOURLY, last:" + hours + ") {pageInfo {totalCost totalConsumption}} production(resolution: HOURLY, last:" + hours + ") {pageInfo {totalProduction totalProfit}}currentSubscription {priceInfo {current {total}}}}}}\" }";
  Serial.println(payload);
  int httpCode = http.POST(payload);
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    //Serial.println(response);
    StaticJsonDocument<200> jsonDoc = parseToJsonDoc(response);
    // Get the "total" value from the response
    double totalCostPerkwh = jsonDoc["data"]["viewer"]["homes"][1]["currentSubscription"]["priceInfo"]["current"]["total"];
    showCost(totalCostPerkwh);
    double totalCost = jsonDoc["data"]["viewer"]["homes"][1]["consumption"]["priceInfo"]["totalCost"];
    double totalConsumption = jsonDoc["data"]["viewer"]["homes"][1]["consumption"]["priceInfo"]["totalConsumption"];
    showConsumtion(totalCost, totalConsumption);
    double totalProduction = jsonDoc["data"]["viewer"]["homes"][1]["consumption"]["priceInfo"]["totalProduction"];
    double totalProfit = jsonDoc["data"]["viewer"]["homes"][1]["production"]["priceInfo"]["totalProfit"];
    showProduction(totalProduction, totalProfit);
  } else {
    Serial.println("something went wrong");
    Serial.println(httpCode);
  }
  http.end();

  // Disconnect from the server
  client.stop();

  int pauseInMilliSeconds = 10*60*1000;
  // Wait a while before making another request
  delay(pauseInMilliSeconds);
}