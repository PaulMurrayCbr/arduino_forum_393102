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
    void (*set_up)(Controller<Effect> *);
    void (*start)(Controller<Effect> *);
    void (*loop)(Controller<Effect> *);
    void (*stop)(Controller<Effect> *);
    void (*tear_down)(Controller<Effect> *);

    Effect(
      void (*set_up)(Controller<Effect> *),
      void (*start)(Controller<Effect> *),
      void (*loop)(Controller<Effect> *),
      void (*stop)(Controller<Effect> *),
      void (*tear_down)(Controller<Effect> *)) :
      set_up(set_up), start(start), loop(loop), stop(stop), tear_down(tear_down) {
    }

    static void default_set_up(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" set_up() ");
#endif
    }

    static void default_start(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" start() ");
#endif
    }

    static void default_loop(Controller<Effect> *controller) {
    }

    static void default_stop(Controller<Effect> *controller) {
#ifdef DEBUG
      Serial.print("Controller ");
      Serial.print((int)controller);
      Serial.println(" stop() ");
#endif
    }

    static void default_tear_down(Controller<Effect> *controller) {
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
};

class StrandEffect: public Effect {
  public:
    StrandEffect(
      void (*set_up)(Controller<Effect> *),
      void (*start)(Controller<Effect> *),
      void (*loop)(Controller<Effect> *),
      void (*stop)(Controller<Effect> *),
      void (*tear_down)(Controller<Effect> *)):
      Effect(set_up, start, loop, stop, tear_down) {
      strandEffects.add(this);
    }

};

class RGBEffect: public Effect {
  public:
    RGBEffect(
      void (*set_up)(Controller<Effect> *),
      void (*start)(Controller<Effect> *),
      void (*loop)(Controller<Effect> *),
      void (*stop)(Controller<Effect> *),
      void (*tear_down)(Controller<Effect> *)):
      Effect(set_up, start, loop, stop, tear_down) {
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
      // common anode - 255 is off
      analogWrite(rPin, 255);
      analogWrite(gPin, 255);
      analogWrite(bPin, 255);
      Controller::off();
    }

    virtual void next() {
      // common anode - 255 is off
      analogWrite(rPin, 255);
      analogWrite(gPin, 255);
      analogWrite(bPin, 255);
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

class Point : StrandEffect {
  public:
    Point() : StrandEffect(default_set_up, default_start, (void (*)(Controller<Effect>*)) Point::loop, default_stop, default_tear_down) {
    }

    // an extremely simple effect that moves the led along by one every 10th of a second.
    static void loop(StrandController *controller) {
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
    Rainbow() : StrandEffect(default_set_up, default_start, (void (*)(Controller<Effect>*)) Rainbow::loop, default_stop, default_tear_down) {
    }

    // an extremely simple effect that moves the led along by one every 10th of a second.
    static  void loop(StrandController *controller) {
      int t =  (controller->ms ) & 255;

      for (int i = 0; i < controller->strand.numPixels(); i++) {
        controller->strand.setPixelColor(i, wheel(controller->strand,  ( (i * 256 / controller->strand.numPixels()) + t) & 255));
      }

      controller->strand.show();
    }

} rainbow;

class LedRainbow : RGBEffect {
  public:
    LedRainbow() : RGBEffect(default_set_up, default_start, (void (*)(Controller<Effect>*)) LedRainbow::loop, default_stop, default_tear_down) {
    }

    static  void loop(RGBController *controller) {
      float t =  (controller->ms ) / 333;

      analogWrite(controller->rPin, (byte) ((sin(t + 0.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * 255.99));
      analogWrite(controller->gPin, (byte) ((sin(t + 1.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * 255.99));
      analogWrite(controller->bPin, (byte) ((sin(t + 2.0 / 3.0 * 2.0 * PI) + 1.0) / 2.0 * 255.99));
    }
} ledRainbow;

class LedFlash : RGBEffect {
  public:
    LedFlash() : RGBEffect(default_set_up, default_start, (void (*)(Controller<Effect>*)) LedFlash::loop, default_stop, default_tear_down) {
    }

    static  void loop(RGBController *controller) {
      int z = (controller->ms / 125)%4;
      
      analogWrite(controller->rPin,z==0?0:255);
      analogWrite(controller->gPin,z==1?0:255);
      analogWrite(controller->bPin,z==2?0:255);

    }

} ledFlash;


// -------------- PINOUT -----------------------


// atttach my four neopixel rings to pins 12-9. Two of my rings are 24-led, and two are 16 led

StrandController s12(24, 11); //, s11(24, 11), s10(16, 10), s9(16, 9);
RGBController led1(3,5,6);

ControllerButton<StrandEffect> analog_0_button(s12, 8);
ControllerButton<RGBEffect> analog_1_button(led1, 9); 

// -------------- main loop -----------------------


void setup() {
#ifdef DEBUG
  Serial.begin(57600);
  while (!Serial);
  Serial.println("\nbegin sketch\n");
#endif

  s12.setup();
  led1.setup();


}

void loop() {
  // put your main code here, to run repeatedly:

  s12.loop();
  led1.loop();

  analog_0_button.loop();
  analog_1_button.loop();

}


