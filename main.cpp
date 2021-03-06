/*
 *
 * Compilation: g++ -std=c++11 -Wall  main.cpp WirelessSignalAdjuster.cpp WirelessSignalAdjuster.h -o adjuster
 * Usage        ./adjuster INTERFACE_NAME UPDATE_INTERVAL_IN_SECONDS
 *
 */
#include "WirelessSignalAdjuster.h"

#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif

WirelessSignalAdjuster* adjuster;


int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: ./adjuster INTERFACE_NAME UPDATE_INTERVAL_IN_SECONDS\n";
        std::exit(EXIT_FAILURE);
    }

    adjuster = new WirelessSignalAdjuster(argv[1], static_cast<unsigned int>(std::atoi(argv[2])));

    adjuster->adjustSignal();

    return 0;
}