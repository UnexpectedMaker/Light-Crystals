## Light Crystals Shipping Firmware
This is the Arduino IDE source code that ships on the Light Crystals PCB n the kit.

The project will continue to be added to and improved as new features are added to the Light Crystals.

#### Arduino IDE Settings
To use this project, you will need to have the following settings in your Arduino IDE:

- Board: "UM OMGS3"
- Port: The COM port that the Light Crystals are connected to.

#### Requirements
This protect requires ArduinoESP32 Core 3.0.5 or later and will not work with 2.x versions of the core.

##### FastLED Library
The FastLED library MUST be the latest MASTER branch downloaded from their [GitHub repo](https://github.com/FastLED/FastLED)
It will not work with the current 3.7.8 release as that is not compatible with ArduinoESP32 Core 3.

##### ArduinoFFT Library
This project requires the ArduinoFFT V1 library to be installed. It is not compatible with the V2 library.