#include <Arduino.h>
// ## Arduino Fan RPM Reading without Interrupts (Polling Method)

// --- Configuration ---
const int TACH_PIN = 2;       // Digital pin connected to the fan's tachometer wire
const unsigned long POLLING_INTERVAL_MS = 1000; // How often to calculate RPM (milliseconds)
const int PULSES_PER_REVOLUTION = 2; // Number of pulses your fan outputs per full revolution (check your fan's datasheet)

// --- Global Variables ---
volatile unsigned long pulseCount = 0;
unsigned long lastReadTime = 0;
unsigned int rpm = 0;
unsigned long lastPulseTime = 0; // To detect if the fan has stopped

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  pinMode(TACH_PIN, INPUT_PULLUP); // Set tachometer pin as input with an internal pull-up resistor
                                  // Use INPUT if you have an external pull-up or if the signal is actively driven high/low
  lastReadTime = millis();
  lastPulseTime = millis();
  Serial.println("Fan RPM Reader Initialized (Polling)");
}

void loop() {
  // --- Pulse Detection (Polling) ---
  // Simple edge detection. You might need to adjust this based on your fan's signal.
  // This example assumes the signal is normally HIGH and pulses LOW.
  static int lastTachState = HIGH; // Assuming pull-up, so default is HIGH
  int currentTachState = digitalRead(TACH_PIN);

  if (lastTachState == HIGH && currentTachState == LOW) { // Falling edge detected
    pulseCount++;
    lastPulseTime = millis(); // Record the time of the latest pulse
  }
  lastTachState = currentTachState;

  // --- RPM Calculation ---
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= POLLING_INTERVAL_MS) {
    // Check if fan has stopped (no pulses for a while longer than the polling interval)
    // The additional 100ms is a small buffer.
    if (currentTime - lastPulseTime > POLLING_INTERVAL_MS + 100 && pulseCount == 0) {
      rpm = 0; // Fan is considered stopped
    } else if (pulseCount > 0) { // Calculate RPM only if pulses were detected
      // Calculate RPM:
      // (pulses / pulses_per_revolution) / (time_interval_seconds) * 60_seconds_per_minute
      rpm = (pulseCount * 60000.0) / (PULSES_PER_REVOLUTION * (currentTime - lastReadTime));
    } else {
      // No pulses within this specific polling interval, but fan might not be fully "stopped" yet
      // according to the lastPulseTime check. Could set to 0 or keep previous non-zero if preferred.
      // For this example, if no pulses in interval, assume 0 for this interval's calculation.
      rpm = 0;
    }

        uint16_t setpoint = analogRead(SETPOINT_PIN);
    setpoint = setpoint >> 2;
    analogWrite(PWM_PIN, setpoint);

    Serial.print("Pulses: ");
    Serial.print(pulseCount);
    Serial.print(" | RPM: ");
    Serial.println(rpm);



    // Reset for next interval
    pulseCount = 0;
    lastReadTime = currentTime;

    // If we reset pulseCount to 0, and the fan had stopped *before* this interval,
    // ensure lastPulseTime reflects that no recent pulses occurred before the next check.
    // However, the current (currentTime - lastPulseTime > POLLING_INTERVAL_MS + 100) check
    // already handles this by seeing a large gap since the *actual* last pulse.
  }

  // You can add other non-blocking code here
}