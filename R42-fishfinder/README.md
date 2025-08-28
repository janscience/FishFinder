# R42 Fishfinder

## Pinout

![pinout](teensy41-R42-fishfinder-pinout.png)


### Amplifier

[Teensy_Amp R4.2](https://github.com/janscience/Teensy_Amp/tree/main/R4.2)


### Display

[Adafruit 2.0" 320x240 Color IPS TFT Display](https://www.adafruit.com/product/4311) on SPI via ST7789 chip:

- MOSI1 (yellow): 11
- CS1 (orange): 10
- RST (green): 9

- SCK1 (white): 13
- D/C (blue) : 14
- BL (purple): 15

- GND (black)
- 3.3V (red)


### Audio amplifier:

[Adafruit MAX98357 I2S Class-D Mono Amp](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp)

- LRC (blue): 20
- BCLK (white): 21
- DIN (yellow): 7
- GAIN: GND (12dB gain)
- SD (green): 6
- GND
- 3.3V


### Switches

- Volume down (purple): 28
- Volume up (blue): 29
- Record (orange): 30
- Voice message (green): 31


### Microphone

[SparkFun Analog MEMS Microphone](https://www.sparkfun.com/products/18011)

- GND (black)
- 3.3V (red)
- A0 (yellow): 41 (A17)


### Real time clock con battery

- GND (black)
- VBAT (red)


### LiPo Charger

[Adafruit LiIon/LiPoly Backpack](https://www.adafruit.com/product/2124)

- VUSB (red)
- Vin  (orange)
- GND (black)

