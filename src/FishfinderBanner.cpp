#include <MicroConfigBanner.h>
#include <TeeRecBanner.h>
#include <FishfinderBanner.h>


void printFishfinderBanner(const char *software, Stream &stream) {
  stream.println("\n========================================================");
  // Generated with figlet http://www.figlet.org
  stream.println(R"( _____ _     _     _____ _           _           )");
  stream.println(R"(|  ___(_)___| |__ |  ___(_)_ __   __| | ___ _ __ )");
  stream.println(R"(| |_  | / __| '_ \| |_  | | '_ \ / _` |/ _ \ '__|)");
  stream.println(R"(|  _| | \__ \ | | |  _| | | | | | (_| |  __/ |   )");
  stream.println(R"(|_|   |_|___/_| |_|_|   |_|_| |_|\__,_|\___|_|   )");
  stream.println();
  if (software != NULL) {
    if (strlen(software) > 10 && strncmp(software, "Fishfinder", 10) == 0)
      software = software + 11;
    stream.print(software);
    stream.print(" ");
  }
  stream.println("by Benda-Lab");
  stream.print("based on ");
  stream.println(FISHFINDER_SOFTWARE);
  stream.print("     and ");
  stream.println(TEEREC_SOFTWARE);
  stream.print("     and ");
  stream.println(MICROCONFIG_SOFTWARE);
  stream.println("--------------------------------------------------------");
  stream.println();
}
