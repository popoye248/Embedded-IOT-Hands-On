//Name : Usama Bin Saeed
//Registration Number: 23-NTU-CS-1094
//MIDS - PROJECT 
//Task A: A button performing three funtions
//        1. Alternative blink of two LEDs
//        2. Both ON 
//        3. PWM
//Task B: A button performing two functions
//        1. Short press (less then 1.5 second) = LED ON
//        2. Long press (more than 1.5 second) = Buzzer ON
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// dr zafar wali gali, madina market, ladhy wala warriach alam chowk 

// Pin definitions
#define BUTTON_A_PIN 32 // For cycling through LED modes
#define BUTTON_B_PIN 33 // For short/long press detection
#define BUZZER_PIN 5    // Buzzer pin
#define LED_PIN1 3      // LED 1
#define LED_PIN2 1      // LED 2
#define LED_PIN3 4      // LED 3 (for PWM fade)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Task A variables
int ledMode = 0; // 0=Both OFF, 1=Alternate blink, 2=Both ON, 3=PWM fade
bool lastButtonAState = HIGH;
bool buttonAState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Task B variables
unsigned long pressStartTime = 0;
bool buttonBPressed = false;
bool lastButtonBState = HIGH;
bool ledBToggleState = false;

// LED animation variables
unsigned long lastBlinkTime = 0;
bool blinkState = false;
int pwmValue = 0;
bool pwmDirection = true;
unsigned long lastPwmTime = 0;

// Function declarations
void handleButtonA();
void handleButtonB();
void handleLEDAnimations();
void resetLEDs();
void playBuzzerTone();
void updateDisplay();

void setup()
{
  Serial.begin(115200); // For debugging

  // Initialize I2C
  Wire.begin(21, 22); // SDA=21, SCL=22

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED failed!");
    for (;;)
      ; // Hang if display fails
  }

  // Initialize pins
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Turn off all LEDs and buzzer initially
  digitalWrite(LED_PIN1, LOW);
  digitalWrite(LED_PIN2, LOW);
  digitalWrite(LED_PIN3, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Display startup message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LED Control System");
  display.setCursor(0, 20);
  display.println("Btn A: Cycle Modes");
  display.setCursor(0, 30);
  display.println("Btn B: Short/Long");
  display.setCursor(0, 40);
  display.println("Ready...");
  display.display();
  delay(2000);

  updateDisplay();
}

void loop()
{
  // Task A: Button A - Cycle through LED modes
  handleButtonA();

  // Task B: Button B - Short/Long press detection
  handleButtonB();

  // Handle LED animations based on current mode
  handleLEDAnimations();

  delay(10);
}

void handleButtonA()
{
  bool reading = digitalRead(BUTTON_A_PIN);

  // Debug button state
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 1000)
  {
    Serial.print("Button A: ");
    Serial.println(reading);
    lastDebug = millis();
  }

  // Check if button state changed
  if (reading != lastButtonAState)
  {
    lastDebounceTime = millis();
  }

  // If stable for debounce period
  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // If button is pressed (LOW) and was previously not pressed
    if (reading == LOW && buttonAState == HIGH)
    {
      // Button A pressed - cycle to next mode
      ledMode = (ledMode + 1) % 4;
      Serial.print("Mode changed to: ");
      Serial.println(ledMode);

      updateDisplay();
      resetLEDs();
    }

    // Update current button state
    buttonAState = reading;
  }

  lastButtonAState = reading;
}

void handleButtonB()
{
  int reading = digitalRead(BUTTON_B_PIN);

  // Button press detection
  if (reading == LOW && lastButtonBState == HIGH)
  {
    // Button just pressed
    buttonBPressed = true;
    pressStartTime = millis();
  }

  // Button release detection
  if (reading == HIGH && lastButtonBState == LOW)
  {
    buttonBPressed = false;
    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration < 1500)
    {
      // Short press - toggle LED
      ledBToggleState = !ledBToggleState;
      digitalWrite(LED_PIN1, ledBToggleState ? HIGH : LOW);

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Button B: Short Press");
      display.setCursor(0, 20);
      display.print("LED: ");
      display.println(ledBToggleState ? "ON" : "OFF");
      display.display();

      Serial.println("Button B: Short Press");
    }
    else
    {
      // Long press - buzzer tone
      playBuzzerTone();

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Button B: Long Press");
      display.setCursor(0, 20);
      display.println("Buzzer Activated!");
      display.display();

      Serial.println("Button B: Long Press");
    }
  }

  lastButtonBState = reading;
}

void handleLEDAnimations()
{
  unsigned long currentTime = millis();

  switch (ledMode)
  {
  case 0: // Both OFF
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    break;

  case 1: // Alternate blink
    if (currentTime - lastBlinkTime >= 500)
    {
      lastBlinkTime = currentTime;
      blinkState = !blinkState;
      digitalWrite(LED_PIN2, blinkState ? HIGH : LOW);
      digitalWrite(LED_PIN3, blinkState ? LOW : HIGH);
    }
    break;

  case 2: // Both ON
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    break;

  case 3: // PWM fade
    if (currentTime - lastPwmTime >= 20)
    {
      lastPwmTime = currentTime;

      // Update PWM value
      if (pwmDirection)
      {
        pwmValue += 5;
        if (pwmValue >= 255)
        {
          pwmValue = 255;
          pwmDirection = false;
        }
      }
      else
      {
        pwmValue -= 5;
        if (pwmValue <= 0)
        {
          pwmValue = 0;
          pwmDirection = true;
        }
      }

      analogWrite(LED_PIN2, pwmValue);
      analogWrite(LED_PIN3, 255 - pwmValue); // Opposite fade
    }
    break;
  }
}

void resetLEDs()
{
  digitalWrite(LED_PIN2, LOW);
  digitalWrite(LED_PIN3, LOW);
  pwmValue = 0;
  pwmDirection = true;
  blinkState = false;
}

void playBuzzerTone()
{
  // Play a tone for 1 second
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void updateDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("=== LED MODE ===");

  display.setCursor(0, 20);
  switch (ledMode)
  {
  case 0:
    display.println("Mode: Both OFF");
    break;
  case 1:
    display.println("Mode: Alternate Blink");
    break;
  case 2:
    display.println("Mode: Both ON");
    break;
  case 3:
    display.println("Mode: PWM Fade");
    break;
  }

  display.setCursor(0, 40);
  display.println("Btn B: Short/Long Press");

  display.display();
}