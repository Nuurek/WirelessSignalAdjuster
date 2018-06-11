//
// Created by nuurek on 11.06.18.
//

#include "WirelessSignalAdjuster.h"

WirelessSignalAdjuster::WirelessSignalAdjuster(const char wirelessInterfaceName[], unsigned int updateInterval) :
    updateInterval(updateInterval) {

    strcpy(request.ifr_name, wirelessInterfaceName);
    request.u.data.pointer = &wirelessInterfaceStatistics;
    request.u.data.flags = 1;
    request.u.data.length = sizeof(wirelessInterfaceStatistics);

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
}

void WirelessSignalAdjuster::adjustSignal() {
    while (true) {
        int ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWSTATS, &request);
        if (ioControlStatus == -1) {
            std::cerr << "Error getting wireless interface statistics: " << strerror(errno) << "\n";
        }

        bool isSignalLevelInDBM = static_cast<bool>(wirelessInterfaceStatistics.qual.updated & IW_QUAL_DBM);
        bool wasSignalLevelUpdated = static_cast<bool>(wirelessInterfaceStatistics.qual.updated &
                                                       IW_QUAL_LEVEL_UPDATED);
        int signalLevel = static_cast<unsigned int>(wirelessInterfaceStatistics.qual.level);
        std::cout << "Signal level " << (isSignalLevelInDBM ? "(in dBm) " : "");
        std::cout << signalLevel;
        std::cout << (wasSignalLevelUpdated ? " (updated)" : "") << "\n";

        sleep(updateInterval);
    }
}
