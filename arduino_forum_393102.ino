/*
    For Bigbroncodiver at http://forum.arduino.cc/index.php?topic=393102.0


   Copyright (c) Paul Murray, 2016. Released into the public domain
   under the unlicense http://unlicense.org .
*/

#define DEBUG


#include <Adafruit_NeoPixel.h>

#include "DebounceInput.h"

// -------------- BUTTONS -----------------------

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
#ifdef DEBUG
        Serial.print("Button ");
        Serial.print((int)this);
        Serial.println(" falling");
#endif
        if (millis() - down > DCLICK_ms) clickcount = 0;
        clickcount ++;
        clickfired = false;
        down = millis();
      }
      else if (rising()) {
#ifdef DEBUG
        Serial.print("Button ");
        Serial.print((int)this);
        Serial.println(" rising");
#endif
        if (!clickfired) {
          onClick(clickcount);
          clickfired = true;
        }
      }
      else if (low() && !clickfired && millis() - down > LONG_CLICK_ms) {
#ifdef DEBUG
        Serial.print("Button ");
        Serial.print((int)this);
        Serial.println(" longclick");
#endif
        onLongClick(clickcount);
        clickfired = true;
      }
    }
};

// -------------- UTILITY CLASS -----------------------

// a simple list stores 20 pointers to whatever.

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
    virtual void set_up(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" set_up() ");
#endif
    }

    virtual void start(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" start() ");
#endif
    }

    virtual void loop(Controller<Effect> *controller) = 0;

    virtual void stop(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" stop() ");
#endif
    }

    virtual void tear_down(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" tear_down() ");
#endif
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    static uint32_t wheel(Adafruit_NeoPixel &strip, byte WheelPos) {
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

    static uint32_t wheel_v(Adafruit_NeoPixel &strip, byte WheelPos, byte v) {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85) {
        return strip.Color((255 - WheelPos * 3) * v / 255, 0, (WheelPos * 3) * v / 255);
      }
      if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, (WheelPos * 3) * v / 255, (255 - WheelPos * 3) * v / 255);
      }
      WheelPos -= 170;
      return strip.Color((WheelPos * 3) * v / 255, (255 - WheelPos * 3) * v / 255, 0);
    }
};

class StrandEffect: public virtual Effect {
  public:
    StrandEffect() {
      strandEffects.add(this);
    }

    void set_up(Controller<Effect> *controller) {
      set_up((StrandController *)controller);
    }

    void start(Controller<Effect> *controller) {
      start((StrandController *)controller);
    }

    void loop(Controller<Effect> *controller) {
      loop((StrandController *)controller);
    }

    void stop(Controller<Effect> *controller) {
      stop((StrandController *)controller);
    }

    void tear_down(Controller<Effect> *controller) {
      tear_down((StrandController *)controller);
    }

    virtual void set_up(StrandController *controller) {
#ifdef DEBUG      
      Effect::set_up((Controller<Effect> *)controller);
#endif      
    }

    virtual void start(StrandController *controller) {
#ifdef DEBUG      
      Effect::start((Controller<Effect> *)controller);
#endif      
    }

    virtual void loop(StrandController *controller) = 0;

    virtual void stop(StrandController *controller) {
#ifdef DEBUG      
      Effect::stop((Controller<Effect> *)controller);
#endif      
    }

    virtual void tear_down(StrandController *controller) {
#ifdef DEBUG      
      Effect::tear_down((Controller<Effect> *)controller);
#endif      
    }



};

class RGBEffect: public virtual Effect {
  public:
    RGBEffect() {
      rgbEffects.add(this);
    }

    
    void set_up(Controller<Effect> *controller) {
      set_up((RGBController *)controller);
    }

    void start(Controller<Effect> *controller) {
      start((RGBController *)controller);
    }

    void loop(Controller<Effect> *controller) {
      loop((RGBController *)controller);
    }

    void stop(Controller<Effect> *controller) {
      stop((RGBController *)controller);
    }

    void tear_down(Controller<Effect> *controller) {
      tear_down((RGBController *)controller);
    }

    virtual void set_up(RGBController *controller) {
#ifdef DEBUG      
      Effect::set_up((Controller<Effect> *)controller);
#endif      
    }

    virtual void start(RGBController *controller) {
#ifdef DEBUG      
      Effect::start((Controller<Effect> *)controller);
#endif      
    }

    virtual void loop(RGBController *controller) = 0;

    virtual void stop(RGBController *controller) {
#ifdef DEBUG      
      Effect::stop((Controller<Effect> *)controller);
#endif      
    }

    virtual void tear_down(RGBController *controller) {
#ifdef DEBUG      
      Effect::tear_down((Controller<Effect> *)controller);
#endif      
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

    // effect set_up and tear_down allows effects to do things like mallocing space and holding them here

    unsigned long prev_ms, ms;
    unsigned long time[5];
    unsigned n[5];
    float f[5];
    void  *p[5];

    boolean isOn = false;
    boolean isInitialized = false;

    virtual void loop() {
      if (!isOn) return;

      ms = millis();
      effects.list[currentEffect]->loop((Controller<Effect> *)this);
      prev_ms = ms;
    }

    virtual void off() {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)this);
      Serial.println(" off");
#endif

      if (!isOn) return;

      ms = prev_ms = millis();
      effects.list[currentEffect]->stop((Controller<Effect> *)this);
      isOn = false;


    }

    virtual void on() {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)this);
      Serial.println(" on");
#endif

      if (isOn) return;

      if (!isInitialized) {
        effects.list[currentEffect]->set_up((Controller<Effect> *)this);
        isInitialized = true;
      }

      ms = prev_ms = millis();
      effects.list[currentEffect]->start((Controller<Effect> *)this);
      isOn = true;
    }

    virtual void next() {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)this);
      Serial.println(" next()");
#endif

      if (isOn) {
        effects.list[currentEffect]->stop((Controller<Effect> *)this);
      }

      if (isInitialized) {
        effects.list[currentEffect]->tear_down((Controller<Effect> *)this);
        isInitialized = false;
      }

      if (++currentEffect >= effects.n) {
        currentEffect = 0;
      }

      if (isOn) {
        effects.list[currentEffect]->set_up((Controller<Effect> *)this);
        isInitialized = true;
        effects.list[currentEffect]->start((Controller<Effect> *)this);
      }
      prev_ms = ms = millis();

    }

};


class RGBController: public Controller<RGBEffect> {
  public:
    int rPin, gPin, bPin;


    RGBController(int rPin, int gPin, int bPin) :
      Controller(rgbEffects),
      rPin(rPin), gPin(gPin), bPin(bPin)
    {
    }

    void setup() {
      pinMode(rPin, OUTPUT);
      pinMode(gPin, OUTPUT);
      pinMode(bPin, OUTPUT);

#ifdef DEBUG
      Serial.print("RgbController ");
      Serial.print((int)this);
      Serial.print(".setup()");
      Serial.println();
#endif
    }


    virtual void off() {
      if (!isOn) return;
      analogWrite(rPin, 0);
      analogWrite(gPin, 0);
      analogWrite(bPin, 0);
      Controller::off();
    }

    virtual void next() {
      analogWrite(rPin, 0);
      analogWrite(gPin, 0);
      analogWrite(bPin, 0);
      Controller::next();
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
      strand.begin();
      strand.clear();
      strand.show();

#ifdef DEBUG
      Serial.print("StrandController ");
      Serial.print((int)this);
      Serial.print(".setup()");
      Serial.println();
#endif
    }

    virtual void off() {
      if (!isOn) return;
      strand.clear();
      strand.show();
      Controller::off();
    }

    virtual void next() {
#ifdef DEBUG
      Serial.print("StrandController ");
      Serial.print((int)this);
      Serial.print(".next()");
      Serial.println();
#endif

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
#ifdef DEBUG
      Serial.print("ControllerButton ");
      Serial.print((int)this);
      Serial.println(" click");
#endif
      controller.next();
    }

    void onLongClick(int) {
#ifdef DEBUG
      Serial.print("ControllerButton ");
      Serial.print((int)this);
      Serial.println(" longclick");
#endif

#ifdef DEBUG
      Serial.print("controller.isOn ");
      Serial.println(controller.isOn);
#endif

      if (controller.isOn) {
        controller.off();
      }
      else {
        controller.on();
      }

#ifdef DEBUG
      Serial.print("controller.isOn ");
      Serial.println(controller.isOn);
#endif
    }
};

// -------------- IMPLEMENTATION OF VARIOUS FUN EFFECTS -----------------------

const int bassPin = 8;

class Point : StrandEffect {
  public:
    void set_up(StrandController *controller) {
      controller->f[0] = 0;
      controller->f[1] = 1;
    }

    // an extremely simple effect that moves the led along by one every 10th of a second.
    void loop(StrandController *controller) {
      if (controller->prev_ms == controller->ms) return;

      int prev = (int) controller->f[0];

      const boolean bass = digitalRead(bassPin) == LOW;
      const int d = controller->ms - controller->prev_ms;

      if (bass) {
        controller->f[1] = 30.0;
      }
      else {
        controller->f[1] -= 1;
        for (int i = 0; i < d; i++)
          controller->f[1] *= .995;
        controller->f[1] += 1;
      }

      controller->f[0] += controller->f[1] / 200;
      while (controller->f[0] >= controller->strand.numPixels()) {
        controller->f[0] -= controller->strand.numPixels();
      }
      if (controller->f[0] < 0) controller->f[0] = 0;

      int now =  (int) controller->f[0];

      if (prev != now) {
        for (int i = 0; i < controller->strand.numPixels(); i +=  controller->strand.numPixels() / 2) {
          controller->strand.setPixelColor((prev) % (controller->strand.numPixels() / 2) + i, 0);
          controller->strand.setPixelColor((now) % (controller->strand.numPixels() / 2) + i, controller->strand.Color(255, 255, 255));
          controller->strand.show();
        }
      }
    }

} point;

class Rainbow : StrandEffect {
  public:
    // an extremely simple effect that moves the led along by one every 10th of a second.
    void loop(StrandController *controller) {
      int t =  (controller->ms / 10) & 255;
      const boolean bass = digitalRead(bassPin) == LOW;

      for (int i = 0; i < controller->strand.numPixels(); i++) {
        controller->strand.setPixelColor(i, wheel_v(controller->strand,  ( (i * 256 / controller->strand.numPixels()) + t) & 255, bass ? 255 : 64));
      }

      controller->strand.show();
    }

} rainbow;

class LedRainbow : RGBEffect {
  public:
    void loop(RGBController *controller) {
      float t =  (controller->ms ) / 333;

      const boolean bass = digitalRead(bassPin) == LOW;

      analogWrite(controller->rPin, (byte) ((sin(t + 0.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * (bass ? 250 : 64)));
      analogWrite(controller->gPin, (byte) ((sin(t + 1.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * (bass ? 250 : 64)));
      analogWrite(controller->bPin, (byte) ((sin(t + 2.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * (bass ? 250 : 64)));
    }
} ledRainbow;

class LedFlash : RGBEffect {
    void loop(RGBController *controller) {
      int z = (controller->ms / 125) % 4;

      const boolean bass = digitalRead(bassPin) == LOW;

      analogWrite(controller->rPin, z == 0 ? bass ? 255 : 64  : 0);
      analogWrite(controller->gPin, z == 1 ? bass ? 255 : 64  : 0);
      analogWrite(controller->bPin, z == 2 ? bass ? 255 : 64  : 0);

    }

} ledFlash;

// -------------- PINOUT -----------------------

// atttach my four neopixel rings to pins 12-9. Two of my rings are 24-led, and two are 16 led

StrandController s12(48, 13); //, s11(24, 11), s10(16, 10), s9(16, 9);
RGBController led1(3, 5, 6);
RGBController led2(9, 10, 11);

ControllerButton<StrandEffect> s12_button(s12, 2);
ControllerButton<RGBEffect> led1_button(led1, 4);
ControllerButton<RGBEffect> led2_button(led2, 7);

// -------------- main loop -----------------------


void setup() {
#ifdef DEBUG
  Serial.begin(57600);
  while (!Serial);
  Serial.println("\nbegin sketch\n");
#endif

  s12.setup();
  led1.setup();
  led2.setup();

  pinMode(bassPin, INPUT_PULLUP);


}



void loop() {
  // put your main code here, to run repeatedly:

  s12.loop();
  led1.loop();
  led2.loop();
  s12_button.loop();
  led1_button.loop();
  led2_button.loop();

}


