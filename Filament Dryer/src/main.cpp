#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the second parameter is how many rows are on your screen
//the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27,  16, 2);


// put function declarations here:
// int myFunction(int, int);

#define PWM_PIN 3
#define LED_PIN 13
#define SETPOINT_PIN A7
#define TACH_PIN 2    // Digital pin connected to the fan's tachometer wire


const unsigned long POLLING_INTERVAL_MS = 1000; // How often to calculate RPM (milliseconds)
const int PULSES_PER_REVOLUTION = 2; // Number of pulses your fan outputs per full revolution (check your fan's datasheet)

// --- Global Variables ---
volatile unsigned long pulseCount = 0;
unsigned long lastReadTime = 0;
unsigned int rpm = 0;
unsigned long lastPulseTime = 0; // To detect if the fan has stopped
unsigned int previousRpm = 1;


volatile static int32_t pulse_cnt, mailbox;
volatile static uint8_t flag;

void ISR_Pulse_Count() {
  pulse_cnt++;
  if (flag) {
    mailbox = pulse_cnt;
    pulse_cnt = 0;
    flag = 0;
  }
}

int32_t get_pulse_cnt(uint32_t timeoutms) {
  int32_t last;
  int32_t startMillis = millis();

  if (!flag) {  // Retrieve the latest data if no request is pending
    last = mailbox;
  }

  flag = 1;  // Request new data. This line is redundant if the flag is already set from a previous iteration.

  while (flag) {
    if (millis() - startMillis > timeoutms) {
      return last;
    }
  }

  return mailbox;

}


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);



  TCCR2B = TCCR2B & B11111000 | B00000001; // set timer 2 divisor to 1 for PWM frequency of 31372.55 Hz

    //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();

  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(TACH_PIN, INPUT_PULLUP); // Set tachometer pin as input with an internal pull-up resistor
                                  // Use INPUT if you have an external pull-up or if the signal is actively driven high/low

  lastReadTime = millis();
  lastPulseTime = millis();
}

// the loop function runs over and over again forever

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

    // tell the screen to write on the top row
  lcd.setCursor(0,0);
  // tell the screen to write “hello, from” on the top  row
  lcd.print("Fan RPM");
  // tell the screen to write on the bottom  row

  if (rpm != previousRpm)
  {
    lcd.setCursor(0, 1);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    // tell the screen to write “Arduino_uno_guy”  on the bottom row
    // you can change whats in the quotes to be what you want  it to be!
    lcd.print(rpm);
    previousRpm = rpm;
  }

}                    // wait for a second


// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }