// Just run the MTP responder on the default SD card.

// You need to:
// git clone git@github.com:KurtE/MTP_Teensy.git
// Requires Teensyduino >=1.57, set USB Type to "Serial + MTP Disk"

#include <SDWriter.h>
#include <MTP_Teensy.h>

SDCard sdcard;


// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 200) {};
  sdcard.begin();
  MTP.begin();
  MTP.addFilesystem(sdcard, "Fishfinder");
}


void loop() {
  MTP.loop();
}
