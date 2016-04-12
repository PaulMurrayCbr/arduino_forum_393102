#include <Adafruit_NeoPixel.h>

#include "DebounceInput.h"

// a Button is constructed on a pin. I knows about clinks=, double clicks, and long clicks.

class Button : DebouncedInput {

  const unsigned long LONG_CLICK_ms = 1000;
  const unsigned long DCLICK_ms = 250;
   
  unsigned long down;
  int clickcount;
  boolean clickfired;

public:
  Button(int pin) : DebouncedInput(pin)  {}

  void (*click_callback)(int);
  void (*longclick_callback)(int);

  void loop() {
    read();

    if(falling()) {
      if(millis() - down > DCLICK_ms) clickcount = 0; 
      clickcount ++;
      clickfired = false;
      down = millis();
    }
    else
    if(rising()) {
      if(!clickfired) {
        if(click_callback) click_callback(clickcount);
        clickfired = true;
      }
    }
    else if(low() && !clickfired && millis() - down > LONG_CLICK_ms) {
      if(longclick_callback) longclick_callback(clickcount);
      clickfired = true;
    }
  }
};


Button b1(4), b2(5), b3(6);

void click(int i) {
      Serial.println("click ");
      Serial.print(i);
      Serial.println();
}

void longclick(int i) {
      Serial.println("longclick ");
      Serial.print(i);
      Serial.println();
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(57600);

  b1.click_callback = click;
  b1.longclick_callback = longclick;
}

void loop() {
  // put your main code here, to run repeatedly:

  b1.loop();
  b2.loop();
  b3.loop();
}


