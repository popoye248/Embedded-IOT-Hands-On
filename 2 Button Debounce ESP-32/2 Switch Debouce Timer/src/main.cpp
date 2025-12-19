// Week3-Lecture3
// Two Switch Debounce Solution using Timer Interrupt
// Embedded IoT System Fall-2025

// Name: Usama Bin Saeed          Reg#: 23-NTU-CS-1094

#include <Arduino.h>

#define LED_PIN1 4     // Pin of first LED
#define BUTTON_PIN1 32 // Button for first LED
#define LED_PIN2 5     // Pin of second LED
#define BUTTON_PIN2 26 // Button for second LED
#define DEBOUNCE_MS 50 // Debounce time
#define DEBOUNCE_US (DEBOUNCE_MS * 1000UL)

hw_timer_t *debounceTimer = nullptr;
volatile bool debounceActive = false; // blocks re-entrant presses while debouncing

// --- One-shot debounce timer ISR ---
// Fires once after DEBOUNCE_MS; confirms press and toggles LED.
void ARDUINO_ISR_ATTR onDebounceTimer()
{
  // If still pressed after debounce window, treat as a valid press
  if (digitalRead(BUTTON_PIN1) == LOW)
  {                                                 // check if button 1 is pressed
    digitalWrite(LED_PIN1, !digitalRead(LED_PIN1)); // invert LED state
  }
  else if (digitalRead(BUTTON_PIN2) == LOW)
  { // check if button 2 is pressed
    digitalWrite(LED_PIN2, !digitalRead((LED_PIN2)));
  }
  debounceActive = false; // allow next press
}

// --- GPIO button ISR ---
// Starts the one-shot debounce timer on a FALLING edge.
void ARDUINO_ISR_ATTR onButtonISR()
{
  if (!debounceActive)
  {
    debounceActive = true;
    // Arm one-shot alarm: autoreload=false, reload_count=0
    timerAlarm(debounceTimer, DEBOUNCE_US, false, 0);
  }
}

void setup()
{
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  digitalWrite(LED_PIN1, LOW);
  digitalWrite(LED_PIN2, LOW);

  // Button to GND with internal pull-up; idle HIGH, pressed LOW
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN1, onButtonISR, FALLING);
  attachInterrupt(BUTTON_PIN2, onButtonISR, FALLING);

  // Create a 1 MHz base timer (1 tick = 1 µs) just for debounce

  debounceTimer = timer_create(100000);
  timerAttachInterrupt(debounceTimer, &onDebounceTimer,true);

  // No alarm yet; we arm it on first button edge
}

void loop()
{
  // Nothing to do. Interrupts handle everything.
}
