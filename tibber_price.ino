#include <WiFi.h>
#include <ArduinoJson.h>  // add this library to your project

// Replace these with your own WiFi and Tibber API credentials
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
IPAddress server(52, 214, 65, 66);  // api.tibber.com

// GraphQL query to get the current price
const char *query = "{viewer {homes {currentSubscription {priceInfo {current {total }}}}}}";

// API token for Tibber demo
const char *token = "5K4MVS-OjfWhK_4yrjOlFe1F6kJXPVf7eQYggo8ebAE";

// Buffer to store the HTTP request
char request[1000];

// Buffer to store the HTTP response
char response[1000];

WiFiClient client;

void setup() {
  // Initialize serial and WiFi
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi!");

  // Connect to the Tibber API server
  Serial.println("Connecting to Tibber API server...");
  if (client.connect(server, 443)) {
    Serial.println("Connected to server!");
  } else {
    Serial.println("Connection failed.");
  }
}

void loop() {
  // Build the HTTP POST request
  sprintf(request, "POST /v1-beta/gql HTTP/1.1\r\n"
                  "Host: api.tibber.com\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %d\r\n"
                  "Connection: close\r\n"
                  "Authorization: Bearer %s\r\n"  // add the API token to the headers
                  "\r\n"
                  "{\"query\":\"%s\"}", strlen(query), token, query);

  // Send the request to the server
  client.print(request);

  // Read the response from the server
  int responseLength = 0;
  while (client.available()) {
    char c = client.read();
    response[responseLength++] = c;
  }

  // Parse the JSON response
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.println
    Serial.println("Error parsing JSON response");
    return;
  }

  // Get the "total" value from the response
  double total = doc["data"]["viewer"]["homes"][0]["currentSubscription"]["priceInfo"]["current"]["total"];

  // Print the total to the serial console
  Serial.println(total);

  // Disconnect from the server
  client.stop();

  // Wait a while before making another request
  delay(10000);
}
