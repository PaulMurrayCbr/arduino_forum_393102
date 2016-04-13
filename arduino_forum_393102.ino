/*
    For Bigbroncodiver at http://forum.arduino.cc/index.php?topic=393102.0


   Copyright (c) Paul Murray, 2016. Released into the public domain
   under the unlicense http://unlicense.org .
*/


#include <Adafruit_NeoPixel.h>

#include "DebounceInput.h"

// -------------- BUTTONS -----------------------

class Button : DebouncedAnalogInput {

    const unsigned long LONG_CLICK_ms = 500;
    const unsigned long DCLICK_ms = 250;

    unsigned long down;
    int clickcount;
    boolean clickfired;

  public:
    Button(int pin) : DebouncedAnalogInput(pin)  {}

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

// -------------- UTILITY CLASS -----------------------

template <class T>
class SimpleList {
  public:
    T *list[20];
    int n = 0;
    void add(T *t) {
      if (n < 20) list[n++] = t;
    }
};

// -------------- EFFECTS AND CONTROLLERS -----------------------


class Effect;
class StrandEffect;
class RGBEffect;

template <class E>
class Controller;
class StrandController;
class RGBController;

SimpleList<StrandEffect> strandEffects;
SimpleList<RGBEffect> rgbEffects;

class Effect
{
  public:
    virtual void start(Controller<Effect> *controller) {}
    virtual void loop(Controller<Effect> *controller) {}
    virtual void stop(Controller<Effect> *controller) {}


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

class StrandEffect: public Effect {
  public:
    StrandEffect() {
      strandEffects.add(this);
    }

};

class RGBEffect: public Effect {
  public:
    RGBEffect() {
      rgbEffects.add(this);
    }
};

template <class E>
class Controller {
  public:
    SimpleList<E> &effects;
    int currentEffect = 0;

    Controller(SimpleList<E> &effects) :
      effects(effects)
    { }

    // these variables are a scratchpad area for the various effects. HOw they are used
    // is defined by the effect. prev_ms and ms are maintained by the loop method here, but are
    // available to the effects

    unsigned long prev_ms, ms;
    unsigned long time[10];
    unsigned n[10];
    float f[10];
    void  *p[10];
    boolean isOn = false;

    void setup() {
      on();
    }

    virtual void loop() {
      if (!isOn) return;
      ms = millis();
      effects.list[currentEffect]->loop((Controller<Effect> *)this);
      prev_ms = ms;
    }

    virtual void off() {
      if (!isOn) return;
      ms = prev_ms = millis();
      effects.list[currentEffect]->stop((Controller<Effect> *)this);
      isOn = false;
    }

    virtual void on() {
      if (isOn) return;
      ms = prev_ms = millis();
      effects.list[currentEffect]->start((Controller<Effect> *)this);
      isOn = true;
    }

    virtual void next() {
      if (isOn) {
        effects.list[currentEffect]->stop((Controller<Effect> *)this);
      }
      if (++currentEffect >= effects.n) {
        currentEffect = 0;
      }
      if (isOn) {
        effects.list[currentEffect]->start((Controller<Effect> *)this);
      }
      prev_ms = ms = millis();
    }

};


class StrandController: public Controller<StrandEffect> {
  public:
    Adafruit_NeoPixel strand;

    StrandController(int strandSize, int strandPin) :
      Controller(strandEffects),
      strand(strandSize, strandPin, NEO_GRB + NEO_KHZ800) {
    }

    void setup() {
      Controller::setup();
      strand.begin();
    }

    void off() {
      if (!isOn) return;
      strand.clear();
      strand.show();
      Controller::off();
    }

    void next() {
      if (!isOn) {
        return;
      }
      strand.clear();
      strand.show();
      Controller::next();
    }

};

template <class E>
class ControllerButton: Button {
    Controller<E> &controller;
  public :
    ControllerButton(Controller<E> &controller, int pin) :
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

// -------------- IMPLEMENTATION OF VARIOUS FUN EFFECTS -----------------------


class Point : StrandEffect {
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

class Rainbow : StrandEffect {
  public:
    void start(StrandController *controller) {}
    void stop(StrandController *controller) {}

    // an extremely simple effect that moves the led along by one every 10th of a second.
    void loop(StrandController *controller) {
      int t =  (controller->ms ) & 255;

      for (int i = 0; i < controller->strand.numPixels(); i++) {
        controller->strand.setPixelColor(i, wheel(controller->strand,  ( (i * 256 / controller->strand.numPixels()) + t) & 255));
      }

      controller->strand.show();
    }

} rainbow;

// -------------- PINOUT -----------------------


// atttach my four neopixel rings to pins 12-9. Two of my rings are 24-led, and two are 16 led

StrandController s12(24, 12), s11(24, 11), s10(16, 10), s9(16, 9);

// attach my three buttons. I will run strips 11 and 10 both off button 5.

ControllerButton<StrandEffect> b12(s12, 4), b11(s11, 5), b10(s10, 5), b9(s9, 6);

// -------------- main loop -----------------------


void setup() {
  // put your setup code here, to run once:

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


