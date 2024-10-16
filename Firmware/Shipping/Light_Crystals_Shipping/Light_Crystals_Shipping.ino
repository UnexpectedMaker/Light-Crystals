//===========================================================================
//
// Light Crystals FFT Example
// Verion 1.0
// October 2024
//
// Written by Seon Rozenblum
// Unexpected Maker
//
// Requires ArduinoESP32 Core 3.0.5 or later
//
// FastLED library MUST be the latest MASTER branch downloaded from their GitHub
// repo https://github.com/FastLED/FastLED It will not work with the
// current 3.7.8 Release as that is not compatible with ArduinoESP32 Core 3
//
// This project requires the ArduinoFFT V1 library to be installed. It is not
// compatible with the V2 library.
//
//===========================================================================

#include "visualisations.h"
#include <ESP_I2S.h>
#include <FastLED.h>
#include <arduinoFFT.h>

I2SClass i2s;

#define NUM_LEDS 81
#define DATA_PIN 6
#define BUT 0
#define BRIGHTNESS 96

uint8_t visual_state = 0;
unsigned long nextButton = 0;
unsigned long nextVisual = 0;

unsigned long nextToggle = 0;

const uint8_t I2S_SCK = 3;
const uint8_t I2S_WS = 2;
const uint8_t I2S_DIN = 4;

// FFT Stuff
int bands[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

constexpr int BLOCK_SIZE = 512;
int32_t raw_samples[BLOCK_SIZE];

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];

// Define a global maximum for normalization over time.
float noiseFloor =
    4.0f; // Start with a reasonable initial guess for the noise floor.
const float minNoiseFloor =
    1.7f; // Minimum level to prevent the noise floor from dropping too low.
const float noiseFloorDecay =
    0.85f; // Controls how quickly the noise floor decays.
const float noiseFloorRise =
    0.05f; // Controls how quickly the noise floor adapts upward.
float raw_divider =
    8388608.0f; // Initial scaling factor to normalize 32-bit input.
float min_raw = 0;
float max_raw = 0;
float _raw = 0;
int maxBandValue = 0;

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
CRGB leds[NUM_LEDS];

// Create an instance of the Visualisations class with the LED array
Visualisations visualisations(leds, NUM_LEDS);

void setup() {
  // we need serial output for the plotter
  Serial.begin(115200);

  delay(3000);

  // Power
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  // User button
  pinMode(0, INPUT_PULLUP);

  // Initialise FastLED

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();

  // Initialise I2S
  i2s.setPins(I2S_SCK, I2S_WS, -1, I2S_DIN);

  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT,
                 I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S bus!");
    return;
  }
}

void loop() {
  if (do_fft_calcs()) {
    if (visual_state == 0) {
      visualisations.displayRippleEffect(bands);
    } else if (visual_state == 1) {
      visualisations.displayVortex(bands);
    } else if (visual_state == 2) {
      visualisations.displayRectanglesFromCenter(bands);
    } else if (visual_state == 3) {
      visualisations.displaySoundReactiveEffect(bands);
    }
    FastLED.show();
  }

  delay(25);

  do_buttons();
}

bool do_fft_calcs() {
  int samples_read =
      i2s.readBytes((char *)raw_samples, sizeof(int32_t) * BLOCK_SIZE);

  if (samples_read != sizeof(raw_samples)) {
    Serial.printf("Only read %d of %d samples, skipping...\n", samples_read,
                  BLOCK_SIZE);
    return false;
  }

  // Prepare the FFT input.
  for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
    vReal[i] =
        (float)raw_samples[i] / (raw_divider / 20); // Normalize the input.
    //    vReal[i] = (float)(raw_samples[i] << 8);
    vImag[i] = 0.0f; // Set imaginary part to zero.

    if (raw_samples[i] > max_raw)
      max_raw = (float)raw_samples[i];
    if (raw_samples[i] < min_raw)
      min_raw = (float)raw_samples[i];
  }

  // Update the raw divider dynamically.
  _raw = max(abs(min_raw), abs(max_raw));
  if (raw_divider < _raw) {
    raw_divider = (float)abs(_raw);
  } else {
    raw_divider = max(raw_divider - 100.0f,
                      1000.0f); // Ensure raw_divider doesn't go too low.
  }

  //  Serial.printf("min: %f, max %f, raw_divider: %f, vReal[2]: %f\n",
  //  abs(min_raw), abs(max_raw), raw_divider, vReal[2]);

  // Apply FFT processing.
  //  FFT.DCRemoval();
  FFT.Windowing(vReal, BLOCK_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_SIZE);

  // Clear bands.
  for (int i = 0; i < 9; i++)
    bands[i] = 0;

  // Calculate raw band values by summing FFT results into bands.
  for (int i = 2; i < (BLOCK_SIZE / 2); i++) {
    float magnitude = vReal[i];

    // Map the FFT bins to the 9 bands.
    if (i <= 2)
      bands[0] += (int)magnitude;
    else if (i <= 4)
      bands[1] += (int)magnitude;
    else if (i <= 6)
      bands[2] += (int)magnitude;
    else if (i <= 14)
      bands[3] += (int)magnitude;
    else if (i <= 28)
      bands[4] += (int)magnitude;
    else if (i <= 50)
      bands[5] += (int)magnitude;
    else if (i <= 120)
      bands[6] += (int)magnitude;
    else if (i <= 190)
      bands[7] += (int)magnitude;
    else
      bands[8] += (int)magnitude;

    //    Serial.printf("%d: %f, ", i, magnitude);
  }
  //  Serial.println();

  // Calculate the total volume as the sum of all band values.
  int totalVolume = 0;
  for (int i = 0; i < 9; i++) {
    totalVolume += bands[i];
  }

  // Update the noise floor dynamically based on the observed volume.
  if (totalVolume > noiseFloor) {
    // If the volume is above the noise floor, let the floor rise slowly.
    noiseFloor =
        min((1.0f - noiseFloorRise) * noiseFloor + noiseFloorRise * totalVolume,
            10.0f);
  } else {
    // If the volume is below the noise floor, decay the floor.
    noiseFloor = max(noiseFloor * noiseFloorDecay, minNoiseFloor);
  }

  // If the total volume is below the noise floor, suppress the visuals.
  if (totalVolume < noiseFloor) {
    for (int i = 0; i < 9; i++) {
      bands[i] = 0; // Zero out the bands to prevent visuals.
    }
  }

  // Normalize each band to the 0-255 range for consistent output.
  //  maxBandValue = 0;
  bool max_set = false;
  int max_band_val = 0;
  for (int i = 0; i < 9; i++) {
    if (bands[i] > max_band_val)
      max_band_val = bands[i];
  }

  if (max_band_val > 0) {
    if (max_band_val > maxBandValue) {
      maxBandValue = max_band_val;
    } else if (maxBandValue > (max_band_val * 3)) {
      maxBandValue = (max_band_val * 2);
    } else if (maxBandValue > (max_band_val * 2)) {
      maxBandValue--;
    }
  }

  //  Serial.printf("SET noiseFloor: %f  maxBandValue: %d, max_band_val:
  //  %d\n\n", noiseFloor, maxBandValue, max_band_val);

  for (int i = 0; i < 9; i++) {
    if (maxBandValue > 0)
      bands[i] = (bands[i] * 255) / maxBandValue; // Scale bands to 0-255.
    bands[i] = constrain(bands[i], 0, 255); // Ensure values stay within range.
    //    Serial.printf("%d ", bands[i]);
  }
  //  Serial.println();

  return true;
}

void do_buttons() {
  if (!digitalRead(BUT)) {
    if (millis() - nextButton > 500) {
      nextButton = millis();

      visual_state++;
      if (visual_state == 4)
        visual_state = 0;
    }
  }
}
