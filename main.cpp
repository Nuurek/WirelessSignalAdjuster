#include "WirelessSignalAdjuster.h"

#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif


int main() {
    WirelessSignalAdjuster adjuster = WirelessSignalAdjuster(IW_NAME, 1);

    adjuster.adjustSignal();

    return 0;
}