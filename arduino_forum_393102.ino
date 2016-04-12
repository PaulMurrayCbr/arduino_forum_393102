/*
 *  For Bigbroncodiver at http://forum.arduino.cc/index.php?topic=393102.0
 * 
 * 
   Copyright (c) Paul Murray, 2016. Released into the public domain
   under the unlicense http://unlicense.org .
*/


#include <Adafruit_NeoPixel.h>

#include "DebounceInput.h"

// a Button is constructed on a pin. I knows about clinks=, double clicks, and long clicks.

class Button : DebouncedInput {

    const unsigned long LONG_CLICK_ms = 500;
    const unsigned long DCLICK_ms = 250;

    unsigned long down;
    int clickcount;
    boolean clickfired;

  public:
    Button(int pin) : DebouncedInput(pin)  {}

    virtual void onClick(int) {}
    virtual void onLongClick(int) {}

    void loop() {
      read();

      if (falling()) {
        if (millis() - down > DCLICK_ms) clickcount = 0;
        clickcount ++;
        clickfired = false;
        down = millis();
      }
      else if (rising()) {
        if (!clickfired) {
          onClick(clickcount);
          clickfired = true;
        }
      }
      else if (low() && !clickfired && millis() - down > LONG_CLICK_ms) {
        onLongClick(clickcount);
        clickfired = true;
      }
    }
};

class StrandController;
class Effect;


int nEffects;
Effect *effects[20]; // this needs to be big enbough for the effect singletons
void register_effect(Effect *e) {
  effects[nEffects++] = e;
}


class Effect
{
  public:
    Effect() {
      register_effect(this);
    }
    virtual void start(StrandController *controller) {}
    virtual void loop(StrandController *controller) {}
    virtual void stop(StrandController *controller) {}


    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t wheel(Adafruit_NeoPixel &strip, byte WheelPos) {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      WheelPos -= 170;
      return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
};


class StrandController {

  public:
    Adafruit_NeoPixel strand;
    int currentEffect = 0;

    // these variables are a scratchpad area for the various effects. HOw they are used
    // is defined by the effect. prev_ms and ms are maintained by the loop method here, but are
    // available to the effects

    unsigned long prev_ms, ms;
    unsigned long time[10];
    unsigned n[10];
    float f[10];
    boolean isOn = false;

    StrandController(int strandSize, int strandPin) :
      strand(strandSize, strandPin, NEO_GRB + NEO_KHZ800) {
    }

    void setup() {
      strand.begin();
      on();
    }

    void loop() {
      if (!isOn) return;
      ms = millis();
      effects[currentEffect]->loop(this);
      prev_ms = ms;
    }

    void off() {
      if (!isOn) return;
      ms = prev_ms = millis();
      effects[currentEffect]->stop(this);
      isOn = false;
      strand.clear();
      strand.show();
    }

    void on() {
      if (isOn) return;
      ms = prev_ms = millis();
      effects[currentEffect]->start(this);
      isOn = true;
    }

    void next() {
      if (isOn) {
        effects[currentEffect]->stop(this);
      }
      if (++currentEffect >= nEffects) {
        currentEffect = 0;
      }
      strand.clear();
      strand.show();
      if (isOn) {
        effects[currentEffect]->start(this);
      }
      prev_ms = ms = millis();
    }

};

class StrandControllerButton: Button {
    StrandController &controller;
  public :
    StrandControllerButton(StrandController &controller, int pin) :
      Button(pin), controller(controller) {
    }

    void loop() {
      Button::loop();
    }

    void onClick(int) {
      controller.next();
    }

    void onLongClick(int) {
      if (controller.isOn) {
        controller.off();
      }
      else {
        controller.on();
      }
    }
};

class Point : Effect {
  public:
    void start(StrandController *controller) {}
    void stop(StrandController *controller) {}

    // an extremely simple effect that moves the led along by one every 10th of a second.
    void loop(StrandController *controller) {
      int a = (controller->prev_ms / 100) % controller->strand.numPixels();
      int b = (controller->ms / 100) % controller->strand.numPixels();

      if (a != b) {
        controller->strand.setPixelColor(a, 0);
        controller->strand.setPixelColor(b, controller->strand.Color(255, 255, 255));
        controller->strand.show();
      }
    }

} point;

class Rainbow : Effect {
  public:
    void start(StrandController *controller) {}
    void stop(StrandController *controller) {}

    // an extremely simple effect that moves the led along by one every 10th of a second.
    void loop(StrandController *controller) {
      int t =  (controller->ms ) & 255;

      for(int i = 0; i<controller->strand.numPixels(); i++) {
        controller->strand.setPixelColor(i, wheel(controller->strand,  ( (i*256/controller->strand.numPixels())+t) & 255));
      }

      controller->strand.show();
    }

} rainbow;

// atttach my four neopixel rings to pins 12-9. Two of my rings are 24-led, and two are 16 led

StrandController s12(24, 12), s11(24, 11), s10(16, 10), s9(16, 9);

// attach my three buttons. I will run strips 11 and 10 both off button 5.

StrandControllerButton b12(s12, 4), b11(s11, 5), b10(s10, 5), b9(s9, 6);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(57600);
  while (!Serial);

  Serial.print(" nEffects is ");
  Serial.print(nEffects);
  Serial.println();

  s12.setup();
  s11.setup();
  s10.setup();
  s9.setup();

}

void loop() {
  // put your main code here, to run repeatedly:

  s12.loop();
  s11.loop();
  s10.loop();
  s9.loop();

  b12.loop();
  b11.loop();
  b10.loop();
  b9.loop();

}


