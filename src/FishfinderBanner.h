/*
  FishfinderBanner - ASCII art banner for serial output.
  Created by Jan Benda, August 25th, 2025.
*/

#ifndef FishfinderBanner_h
#define FishfinderBanner_h


#include <Arduino.h>


#define FISHFINDER_SOFTWARE "Fishfinder version 2.0.0"


void printFishfinderBanner(const char *software=NULL, Stream &stream=Serial);


#endif

