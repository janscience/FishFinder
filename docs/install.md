# Installation

## Arduino IDE

Install the [Arduino IDE](https://www.arduino.cc/en/software) with
[Teensy
support](https://www.pjrc.com/arduino-ide-2-0-0-teensy-support/) as
described
[here](https://github.com/janscience/TeeRec/blob/main/docs/install.md#requirements).


## TeeRec library

Install the [TeeRec library](https://github.com/janscience/TeeRec) and
its dependencies directly from the [library
manager](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library)
of the [Arduino IDE](https://docs.arduino.cc/software/ide-v2).

If you prefer to install the [TeeRec
library](https://github.com/janscience/TeeRec) from its [github
repository](https://github.com/janscience/TeeRec), because you want to
get the latest updates, follow the [TeeRec installation
instructions](https://github.com/janscience/TeeRec/blob/main/docs/install.md).


## FishFinder installation

Clone the [FishFinder](https://github.com/janscience/FishFinder) repository
directly into 'Arduino/libraries':
```sh
cd Arduino/libraries
git clone https://github.com/janscience/FishFinder.git
```

Alternatively, download the whole repository as a zip archive (open
https://github.com/janscience/FishFinder in your browser and click on the
green "Code" button). Unpack the zip file:
```sh
cd Arduino/libraries
unzip ~/Downloads/FishFinder-main.zip
```

If you want to edit the FishFinder files, mark the library as developmental:
```sh
cd Arduino/libraries/FishFinder
touch .development
```

Close the Arduino IDE and open it again. Then the Arduino IDE knows
about the FishFinder library and its examples.

