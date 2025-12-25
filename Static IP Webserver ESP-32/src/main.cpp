/*
Usama Bin Saeed    23-NTU-CS-1094

  Basic static IP ESP32 Webserver
  Control Built-in LED (GPIO 2)
*/

#include <WiFi.h>

const char *ssid = "HiddenWiki";
const char *password = "CANADA121";

IPAddress local_IP(192, 168, 0, 157);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);


WiFiServer server(80);

const int LED_PIN = 2;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Configuring Static IP...");
  if (!WiFi.config(local_IP, gateway, subnet, dns1))
  {
    Serial.println("⚠ Static IP Configuration Failed!");
  }
  else
  {
    Serial.println("Static IP Configured Successfully.");
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 Static IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  WiFiClient client = server.available();
  if (!client)
    return;

  Serial.println("New Client Connected");

  String request = client.readStringUntil('\r');
  Serial.println(request);

  if (request.indexOf("/LED=ON") != -1)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  if (request.indexOf("/LED=OFF") != -1)
  {
    digitalWrite(LED_PIN, LOW);
  }

  String htmlPage =
      "<!DOCTYPE html><html>"
      "<h1>ESP32 LED Control</h1>"
      "<p><a href=\"/LED=ON\"><button>LED ON</button></a></p>"
      "<p><a href=\"/LED=OFF\"><button>LED OFF</button></a></p>"
      "</html>";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(htmlPage);

  delay(1);
  client.stop();
  Serial.println("Client Disconnected");
}