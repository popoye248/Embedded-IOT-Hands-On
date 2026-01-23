#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ==================== GLOBAL VARIABLES ====================

char auth[] = "Eaiiq3nJ0nAMRzsCfq4YZjPVwsxtTQSD";
char ssid[] = "shezuka";
char pass[] = "abcdefgi";

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
#define SMOKE_SENSOR_PIN 34
#define LDR_PIN 35
#define DHTPIN 4
#define BUZZER_PIN 19
#define RED_LED_PIN 5
#define BUTTON_PIN 18

// Sensor Thresholds (ADJUST THESE!)
#define SMOKE_THRESHOLD 1800 // Smoke level for fire
#define LDR_THRESHOLD 3000   // Light level for fire
#define TEMP_THRESHOLD 25.0  // Temperature for fire (°C)
#define HUM_THRESHOLD 15.0   // Humidity drop for fire (%)

// DHT Sensor
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensor Values
int smokeValue = 0;
int ldrValue = 0;
float temperature = 0;
float humidity = 0;

// System States
bool isFireDetected = false;
bool isAlarmActive = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// Normal humidity reference
float normalHumidity = 60.0;

// ==================== PROTOTYPE OF FUNCTIONS ====================
void ReadSmokeSensor();
void ReadLDR();
void ReadDHT();
void NormalDisplay();
void AlertDisplay();
void LEDBlinking();
void BuzzerON();
void BuzzerOFF();
void ResetAlarm();
void Connections();
void continuousChecking();
void BlynkPassing();

// ==================== SENSOR READING FUNCTIONS ====================
void ReadSmokeSensor()
{
  smokeValue = analogRead(SMOKE_SENSOR_PIN);

  if (smokeValue > SMOKE_THRESHOLD)
  {
    Serial.print("High smoke detected: ");
    Serial.println(smokeValue);
    ReadDHT(); // Check temperature and humidity
  }
  else
  {
    BlynkPassing();
    NormalDisplay();
  }
}

void BlynkPassing()
{
  // Update Blynk if connected
  if (Blynk.connected())
  {
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, smokeValue);
    Blynk.virtualWrite(V3, ldrValue);
    Blynk.virtualWrite(V4, 0); // LED off
  }
}

void ReadLDR()
{
  ldrValue = analogRead(LDR_PIN);

  if (ldrValue > LDR_THRESHOLD)
  {
    Serial.print("High light detected: ");
    Serial.println(ldrValue);
    ReadDHT(); // Check temperature and humidity
  }
  else
  {
    BlynkPassing();
    NormalDisplay();
  }
}

void ReadDHT()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Check if readings failed
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 25.0;
    humidity = 50.0;
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("°C, Hum: ");
  Serial.print(humidity);
  Serial.println("%");

  // Check if both temperature is high AND humidity dropped
  bool tempHigh = temperature > TEMP_THRESHOLD;
  bool humDrop = (normalHumidity - humidity) > HUM_THRESHOLD;

  if (tempHigh || humDrop)
  {
    Serial.println("FIRE DETECTED: High temp + humidity drop!");
    AlertDisplay();
  }
  else
  {
    BlynkPassing();
    NormalDisplay();
  }
}

// ==================== DISPLAY FUNCTIONS ====================
void NormalDisplay()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 0);
  display.println("All safe.");
  display.setTextSize(1);
  display.setCursor(5, 16);
  display.println("Have a normal Day :)");

  display.setCursor(0, 22);
  display.println("--------------------");

  display.setCursor(0, 32);
  display.print("Temp:");
  display.print(temperature, 1);
  display.print("C \n");
  display.print("Hum:");
  display.print(humidity, 0);
  display.print("% \n");
  display.print("Smoke:");
  display.print(smokeValue);
  display.print("\n");
  display.print("Light: ");
  display.print(ldrValue);

  display.display();
}

void AlertDisplay()
{
  isFireDetected = true;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 0);
  display.println("EVACUATE");
  display.setTextSize(1);
  display.setCursor(5, 16);
  display.println("THE BUILDING");

  display.setCursor(0, 22);
  display.println("--------------------");

  display.setCursor(0, 32);
  display.print("Temp:");
  display.print(temperature, 1);
  display.print("C \n");
  display.print("Hum:");
  display.print(humidity, 0);
  display.print("% \n");
  display.print("Smoke:");
  display.print(smokeValue);
  display.print("\n");
  display.print("Light: ");
  display.print(ldrValue);

  display.display();

  // Activate alarm
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(RED_LED_PIN, HIGH);

  // Update Blynk
  if (Blynk.connected())
  {
    Blynk.virtualWrite(V4, 255); // LED on
  }

  Serial.println("ALARM ACTIVATED!");
}

// ==================== ALARM CONTROL FUNCTIONS ====================
void LEDBlinking()
{
  if (isAlarmActive && millis() - lastBlinkTime > 500)
  {
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(RED_LED_PIN, ledState);
  }
}

void BuzzerON()
{
  digitalWrite(BUZZER_PIN, HIGH);
}

void BuzzerOFF()
{
  digitalWrite(BUZZER_PIN, LOW);
}

void ResetAlarm()
{
  isFireDetected = false;
  isAlarmActive = false;
  BuzzerOFF();
  digitalWrite(RED_LED_PIN, LOW);
  NormalDisplay();

  if (Blynk.connected())
  {
    Blynk.virtualWrite(V4, 0); // LED off
  }

  Serial.println("Alarm Reset");
}

// ==================== CONNECTION FUNCTIONS ====================
void Connections()
{
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nWiFi Connection Failed!");
  }

  // Connect to Blynk
  Serial.print("Connecting to Blynk...");
  Blynk.begin(auth, ssid, pass);

  delay(2000); // Wait for connection

  if (Blynk.connected())
  {
    Serial.println("Blynk Connected!");
  }
  else
  {
    Serial.println("Blynk Connection Failed!");
  }
}

// ==================== SETUP FUNCTION ====================
void setup()
{
  Serial.begin(115200);

  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Turn off buzzer and LED initially
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("OLED display failed!");
    while (1)
      ; // Stop if OLED fails
  }

  // Show startup screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("IoT Shield-X");
  display.setCursor(5, 35);
  display.println("Fire Alarm System");
  display.display();
  delay(3000);

  // Make connections
  Connections();

  // Get initial humidity reading
  delay(2000);
  normalHumidity = dht.readHumidity();
  if (isnan(normalHumidity))
    normalHumidity = 60.0;

  Serial.println("System Setup Complete!");
  Serial.print("Normal humidity set to: ");
  Serial.print(normalHumidity);
  Serial.println("%");
}

// ==================== CONTINUOUS CHECKING ====================
void continuousChecking()
{
  ReadSmokeSensor();
  ReadLDR();
  ReadDHT();

  // Handle alarm blinking if active
  if (isFireDetected)
  {
    isAlarmActive = true;
    LEDBlinking();
  }
}

// ==================== MAIN LOOP ====================
void loop()
{
  Blynk.run(); // Keep Blynk connection alive

  continuousChecking();

  // Check if alarm should auto-reset (after 10 seconds)
  static unsigned long alarmStartTime = 0;
  if (isAlarmActive)
  {
    if (alarmStartTime == 0)
    {
      alarmStartTime = millis();
    }

    if (millis() - alarmStartTime > 10000)
    { // 10 seconds
      ResetAlarm();
      alarmStartTime = 0;
    }
  }
  else
  {
    alarmStartTime = 0;
  }

  delay(100); // Small delay
}



/*
Libraries

Defination

Global Variables

void ReadSmokeSensor(){
smoke value
if high{
ReadDHT()
}else{
NormalDisplay()}
}

void ReadLDR(){
LDR value
if high{
ReadDHT()
}else{
NormalDisplay()
}
}

void ReadDHT(){
temp value
hum value
if high && high{
AlertDisplay()
}else{
NormalDisplay()
}
}

void NormalDisplay(){
displayOnOLED("All safe. Have a normal Day:)")
}

void AlertDisplay(){
displayOnOLED(Evacuate the building)
LEDBlinking();
BuzzerON();
}

void Connections(){
WiFi.Connect(ssid,pass)
Blynk.Connect(auth, ssid, pass)
}

void Setup(){
all setup including LED, BUZZER, LDR, DHT, etc
displayOnOLED("IoT Shield-X Fire Alarm System")
Connections();
}

void continuousChecking(){
ReadSmokeSensor();
ReadLDR();
ReadDHT();
}

void loop(){
continuousChecking();
}
*/
