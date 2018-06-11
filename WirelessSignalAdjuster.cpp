//
// Created by nuurek on 11.06.18.
//

#include "WirelessSignalAdjuster.h"

WirelessSignalAdjuster::WirelessSignalAdjuster(const char wirelessInterfaceName[], unsigned int updateInterval) :
        updateInterval(updateInterval) {

    strncpy(rangeRequest.ifr_name, wirelessInterfaceName, IFNAMSIZ);
    rangeRequest.u.data.pointer = &wirelessInterfaceRange;
    rangeRequest.u.data.flags = 0;
    rangeRequest.u.data.length = sizeof(wirelessInterfaceRange);

    strncpy(statisticsRequest.ifr_name, wirelessInterfaceName, IFNAMSIZ);
    statisticsRequest.u.data.pointer = &wirelessInterfaceStatistics;
    statisticsRequest.u.data.flags = 1;
    statisticsRequest.u.data.length = sizeof(wirelessInterfaceStatistics);

    socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFileDescriptor == -1) {
        std::cerr << "Error creating datagram socket: " << strerror(errno) << "\n";
        std::exit(EXIT_FAILURE);
    }
}

WirelessSignalAdjuster::~WirelessSignalAdjuster() {
    int status = close(this->socketFileDescriptor);
    if (status == -1) {
        std::cerr << "Error closing datagram socket: " << strerror(errno) << "\n";
    } else {
        std::cout << "Closed datagram socket\n";
    }
    socketFileDescriptor = 0;
}

void WirelessSignalAdjuster::adjustSignal() {
    while (socketFileDescriptor) {
        int ioControlStatus;

        ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWRANGE, &rangeRequest);
        if (ioControlStatus == -1) {
            std::cerr << "Error getting wireless interface range: " << strerror(errno) << "\n";
        }
        int maxSignalQuality = wirelessInterfaceRange.max_qual.qual;
        int maxSignalLevel = wirelessInterfaceRange.max_qual.level;

        ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWSTATS, &statisticsRequest);
        if (ioControlStatus == -1) {
            std::cerr << "Error getting wireless interface statistics: " << strerror(errno) << "\n";
        }

        bool isSignalLevelInDBM = static_cast<bool>(wirelessInterfaceStatistics.qual.updated & IW_QUAL_DBM);
        bool wasSignalLevelUpdated = static_cast<bool>(
                wirelessInterfaceStatistics.qual.updated & IW_QUAL_LEVEL_UPDATED
        );

        if (wasSignalLevelUpdated) {
            int signalQuality, signalLevel;

            signalQuality = wirelessInterfaceStatistics.qual.qual;

            if (isSignalLevelInDBM) {
                signalLevel = wirelessInterfaceStatistics.qual.level - 0x100;
            } else {
                signalLevel = wirelessInterfaceStatistics.qual.level;
            }
            std::cout << "Signal quality: " << signalQuality << "/" << maxSignalQuality;
            std::cout << ", level: ";

            if (isSignalLevelInDBM) {
                std::cout << signalLevel << "[dBm]";
            } else {
                std::cout << signalLevel << "/" << maxSignalLevel;
            }
            std::cout << "\n";
        }

        sleep(updateInterval);
    }
}
