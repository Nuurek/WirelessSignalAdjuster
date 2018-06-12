#include "signal.h"
#include "WirelessSignalAdjuster.h"

#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif

WirelessSignalAdjuster* adjuster;

void signalHandler(int signal) {
    delete adjuster;
}


int main(int argc, char** argv) {
    signal(SIGTERM, signalHandler);

    if (argc < 3) {
        std::cout << "Usage: ./adjuster INTERFACE_NAME UPDATE_INTERVAL_IN_SECONDS\n";
        std::exit(EXIT_FAILURE);
    }

    adjuster = new WirelessSignalAdjuster(argv[1], static_cast<unsigned int>(std::atoi(argv[2])));

    adjuster->adjustSignal();

    return 0;
}