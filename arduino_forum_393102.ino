/*
   Copyright (c) Paul Murray, 2016. Released into the public domain
   under the unlicense http://unlicense.org .
*/


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

    virtual void onClick(int);
    virtual void onLongClick(int);

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

class Effect
{
  protected:
    Effect();
  public:
    virtual void start(StrandController *controller);
    virtual void loop(StrandController *controller);
    virtual void stop(StrandController *controller);
};



class StrandController {
    static int nEffects;
    static Effect *effects[20]; // this needs to be big enbough for the effect singletons

    Adafruit_NeoPixel strand;
    int currentEffect = 0;

  public:

    static void register_effect(Effect *e) {
      effects[nEffects++] = e;
    }


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
      if (isOn) {
        effects[currentEffect]->start(this);
      }
      prev_ms = ms = millis();
    }

};

Effect::Effect() {
  StrandController::register_effect(this);
}

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






void setup() {
  // put your setup code here, to run once:

  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:

}


