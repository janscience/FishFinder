# Hardware

![hardware](images/fishfinder-v1a-hardware.png)

- Amplifier: [Teensy_Amp
  R3.0](https://github.com/janscience/Teensy_Amp/tree/main/R3.0)
- Microcontroller: [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)
- Battery charger: [Adafruit LiIon/LiPoly
  Backpack](https://www.adafruit.com/product/2124)
- Battery (below the PCB): [3.7V 2000mAh Lithium Polymer
  Akku](https://www.exp-tech.de/zubehoer/batterien-akkus/lipo-akkus/6329/3.7v-2000mah-lithium-polymer-akku-mit-jst-ph-anschluss?c=1191)
- Audio amplifier: [Adafruit MAX98357 I2S Class-D Mono
  Amp](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp)
- Speaker: [Adafruit mono enclosed speaker
  3W](https://www.adafruit.com/product/3351)
- Display: [Adafruit 2.0" 320x240 Color IPS TFT
  Display](https://www.adafruit.com/product/4311)
- Microphone: [SparkFun Analog MEMS
  Microphone](https://www.sparkfun.com/products/18011)
- USB connector
- Mono jack socket
- Banana socket
- Coin battery holder
- On/off switch
- Volume up/down switch
- Record button
- Voce-message button


## Pinout

![pinout](images/fishfinder-teensy3.5-pinout.png)


### Amplifier

A2, A10

### Display

On SPI1:

- MOSI1: 0
- D/C: 10
- RST: 1
- SCK1: 32
- CS1: 31
- BL: 30

- GND
- 3.3V

### Audio amplifier:

- LRC: 23
- BCLK: 9
- DIN: 22
- GAIN: GND
- SD: 7

- GND
- 3.3V

### Switches

- Volume down: 26
- Volume up: 27
- Record: 28
- Voice message: 29


### LEDs

Connect LEDs to 150Ohm resistances.

- Record: 11
- Voice: 12
