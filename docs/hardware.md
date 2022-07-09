# Hardware

![hardware](images/fishfinder-v1a-hardware.png)


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
