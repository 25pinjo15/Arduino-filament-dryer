// # we will try to make this dryer now !

#include <Arduino.h>

#include <ItemList.h>
#include <ItemRange.h>
#include <ItemToggle.h>
#include <LcdMenu.h>
#include <MenuScreen.h>
#include <SimpleRotary.h>
#include <display/LiquidCrystal_I2CAdapter.h>
#include <input/SimpleRotaryAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>

#define LCD_ROWS 2
#define LCD_COLS 16

// Declare the callbacks
void callback(int pos);
void toggleBacklight(bool isOn);
void toggleSOS(bool isOn);

std::vector<const char *> colors = {"Red", "Green", "Blue", "Orange", "Aqua", "Yellow", "Purple", "Pink"};

// clang-format off
MENU_SCREEN(mainScreen, mainItems,
    ITEM_BASIC("Connect to WiFi and long string !!"),
    ITEM_LIST("Color", colors, [](const uint8_t color) { Serial.println(colors[color]); }),
    ITEM_TOGGLE("Blink SOS", toggleSOS),
    ITEM_RANGE("Dist", 0, 1, 0, 50, callback, "%dm", 1, true),
    ITEM_TOGGLE("Backlight", toggleBacklight),
    ITEM_BASIC("Blink random"));
// clang-format on

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, LCD_COLS, LCD_ROWS);
LcdMenu menu(renderer);
SimpleRotary encoder(2, 3, 4);
SimpleRotaryAdapter rotaryInput(&menu, &encoder);




// *START  SETUP START ***

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    renderer.begin();
    menu.setScreen(mainScreen);
    // encoder.setTrigger(HIGH);

}


// *END SETUP END ***



// *START  LOOP START ***

void loop()
{
    rotaryInput.observe();
    // check if string on menu is to long for display and scroll it

    
}

// *END  LOOP END ***


// *START  CALLBACKS & METHODs START ***

// Define the callbacks
void toggleBacklight(bool isOn)
{
    lcdAdapter.setBacklight(isOn);
}

void toggleSOS(bool isOn)
{
    // make the built-in LED blink SOS in Morse code
    if (isOn) {
        // S: dot dot dot
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(250);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
        delay(500);
        // O: dash dash dash
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(750);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
        delay(500);
        // S: dot dot dot
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(250);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
        isOn = false; // Reset the flag to stop continuous blinking
    } else {
        digitalWrite(LED_BUILTIN, LOW); // Turn off the LED if not blinking SOS
    }
}

void callback(int pos)
{
    Serial.println(pos);
}