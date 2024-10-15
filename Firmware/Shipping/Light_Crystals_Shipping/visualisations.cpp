#include "visualisations.h"

Visualisations::Visualisations(CRGB* ledArray, int numLeds) : leds(ledArray), num_leds(numLeds) { }


void Visualisations::displayVortex(int* bands)
{
  fadeToBlackBy( leds, num_leds, 30);

  // Calculate the overall volume from all bands.
  int totalVolume = 0;
  for (int i = 0; i < 9; i++) {
    totalVolume += bands[i];
  }
  int vortexSpeed = map(totalVolume, 0, 2300, 1, 20); // Adjust speed based on volume.

  // Draw the vortex using polar coordinates.
  for (int radius = 0; radius < 5; radius++) {
    for (int band = 0; band < 9; band++) {
      int x = 4 + radius * cos((angle + band * 20) * PI / 180); // Calculate x using cosine.
      int y = 4 + radius * sin((angle + band * 20) * PI / 180); // Calculate y using sine.

      // Make sure coordinates are within bounds.
      if (x >= 0 && x < 9 && y >= 0 && y < 9) {
        int ledIndex = y * 9 + x;
        leds[ledIndex] = CHSV((band * 28 + radius * 10) % 255, 255, 255);
      }
    }
  }

  angle += vortexSpeed; // Rotate the vortex.
  angle %= 360; // Keep the angle within 0-360.
}

void Visualisations::displayRippleEffect(int* bands)
{
  fadeToBlackBy(leds, num_leds, 60);

  // Iterate over each band.
  for (int band = 0; band < 9; band++) {
    int value = bands[band];  // Get the sound value for the band (0-255).

    // Use the value to adjust the ripple's radius.
    if (value > 50) {  // Trigger a new ripple if the value is above a threshold.
      rippleRadius[band] = 1; // Reset the radius to start a new ripple.
    }

    // Update the ripple radius and render it.
    if (rippleRadius[band] > 0) {
      drawRipple(band, rippleRadius[band], CHSV(band * 28, 255, 255));
      rippleRadius[band] += rippleSpeed;  // Increase the radius for the next frame.

      // Fade the ripple out gradually.
      if (rippleRadius[band] > 9 + rippleDecay) {
        rippleRadius[band] = 0; // Stop the ripple when it fades out.
      }
    }
  }
}

// Function to draw a circular ripple.
void Visualisations::drawRipple(int band, int radius, CRGB color)
{
  // Calculate the origin of the ripple based on the band.
  int originX = band; // This spreads the ripple origins along the x-axis.
  int originY = 4; // Center the ripple vertically at y = 4 (middle row).

  // Iterate over all pixels in the 9x9 grid.
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      int distance = sqrt(sq(x - originX) + sq(y - originY)); // Calculate distance from the origin.
      if (distance == radius) { // If the distance matches the radius, color the pixel.
        int ledIndex = y * 9 + x;
        leds[ledIndex] = color;
      }
    }
  }
}

void Visualisations::displaySoundReactiveEffect(int* bands)
{
  fadeToBlackBy(leds, num_leds, 50);

  // Iterate over each band (column).
  for (int col = 0; col < 9; col++) {
    int value = bands[col];  // Get the sound value for the band (0-255).

    // Map the band value to a height (0-9 pixels).
    int height = map(value, 0, 255, 0, 9);

    // Light up the column from bottom to top.
    for (int row = 0; row < height; row++) {
      int ledIndex = (8 - row) * 9 + col;  // Calculate the LED index in the 9x9 matrix.
      leds[ledIndex] = CHSV(col * 28, 255, 255);  // Use a different color for each column.
    }
  }
}


void Visualisations::displayRectanglesFromCenter(int* bands)
{
  // Fade out previous visuals slightly for a smoother transition.
  fadeToBlackBy(leds, num_leds, 10);

  // Define the center point and maximum size.
  int centerX = 4;
  int centerY = 4;
  int maxSize = 4; // Maximum size is 4, so we can draw up to 5 rectangles (0-4).

  // Loop over each band and draw a corresponding rectangle from center outward.
  for (int band = 0; band <= maxSize; band++) {
    // Calculate the bounds of the current rectangle.
    int minX = centerX - band;
    int maxX = centerX + band;
    int minY = centerY - band;
    int maxY = centerY + band;

    // Get the brightness or color for this band (based on the band value).
    uint8_t brightness = bands[band] * 3;

    // Set the color for this rectangle.
    CRGB color = CHSV(band * 30, 255, brightness);

    // Draw the top and bottom edges of the rectangle.
    for (int x = minX; x <= maxX; x++) {
      if (minY >= 0 && minY < 9) leds[minY * 9 + x] = color;  // Top edge.
      if (maxY >= 0 && maxY < 9) leds[maxY * 9 + x] = color;  // Bottom edge.
    }

    // Draw the left and right edges of the rectangle.
    for (int y = minY + 1; y < maxY; y++) { // Avoid double-drawing corners.
      if (minX >= 0 && minX < 9) leds[y * 9 + minX] = color;  // Left edge.
      if (maxX >= 0 && maxX < 9) leds[y * 9 + maxX] = color;  // Right edge.
    }
  }
}
