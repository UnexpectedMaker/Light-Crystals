#ifndef VISUALISATIONS_H
#define VISUALISATIONS_H

#include <FastLED.h>

class Visualisations {
  public:
    Visualisations(CRGB* ledArray, int numLeds);
    void displayVortex(int* bands);
    void displaySoundReactiveEffect(int* bands);
    void displayRippleEffect(int* bands);
    void displayRectanglesFromCenter(int* bands);

  private:
    CRGB* leds;
    int num_leds;

    void drawRipple(int band, int radius, CRGB color);

    int angle = 0;
    int rippleRadius[9] = {0}; // Store the radius of each band-originated ripple.
    int rippleSpeed = 1; // Speed at which ripples expand.
    int rippleDecay = 15; // How quickly the ripple fades out (lower is slower).
};

#endif
