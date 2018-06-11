#include "signal.h"
#include "WirelessSignalAdjuster.h"

#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif

WirelessSignalAdjuster* adjuster;

void signalHandler(int signal) {
    delete adjuster;
}


int main() {
    signal(SIGTERM, signalHandler);

    adjuster = new WirelessSignalAdjuster(IW_NAME, 1);

    adjuster->adjustSignal();

    return 0;
}