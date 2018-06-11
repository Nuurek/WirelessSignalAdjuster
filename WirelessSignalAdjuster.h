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
    struct iw_range wirelessInterfaceRange;
    struct iwreq rangeRequest;

    struct iw_statistics wirelessInterfaceStatistics;
    struct iwreq statisticsRequest;

public:
    WirelessSignalAdjuster(const char wirelessInterfaceName[], unsigned int updateInterval);
    ~WirelessSignalAdjuster();

    void adjustSignal();
};


#endif //WIRELESSSIGNALADJUSTER_WIRELESSSIGNALADJUSTER_H
