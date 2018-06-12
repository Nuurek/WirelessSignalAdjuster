//
// Created by nuurek on 11.06.18.
//

#ifndef WIRELESSSIGNALADJUSTER_WIRELESSSIGNALADJUSTER_H
#define WIRELESSSIGNALADJUSTER_WIRELESSSIGNALADJUSTER_H

#include <sys/socket.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <zconf.h>
#include <sys/ioctl.h>

#include <cstring>
#include <iostream>


class WirelessSignalAdjuster {
private:
    unsigned int updateInterval;
    int socketFileDescriptor;
    struct iw_range wirelessInterfaceParameters;
    struct iwreq rangeRequest;

    struct iw_statistics wirelessInterfaceStatistics;
    struct iwreq statisticsRequest;

    struct iwreq txPowerRequest;

    int powerDataUnit;
    int defaultTXPower;
    int minSignalLevel, maxSignalLevel;
    size_t maxTXPowerNumber = 0;
    int maxTXPower[IW_MAX_TXPOWER];

    void getSignalParameters();
    bool getSignalStatistics();
    int getRXLevel();
    float calculateRXLevelRatio(int level);
    int getTXLevel();

public:
    WirelessSignalAdjuster(const char wirelessInterfaceName[], unsigned int updateInterval);
    ~WirelessSignalAdjuster();

    void adjustSignal();
};


#endif //WIRELESSSIGNALADJUSTER_WIRELESSSIGNALADJUSTER_H
