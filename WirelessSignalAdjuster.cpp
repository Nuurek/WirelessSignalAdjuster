//
// Created by nuurek on 11.06.18.
//

#include "WirelessSignalAdjuster.h"

WirelessSignalAdjuster::WirelessSignalAdjuster(const char wirelessInterfaceName[], unsigned int updateInterval) :
        updateInterval(updateInterval) {

    strncpy(rangeRequest.ifr_name, wirelessInterfaceName, IFNAMSIZ);
    rangeRequest.u.data.pointer = &wirelessInterfaceParameters;
    rangeRequest.u.data.flags = 0;
    rangeRequest.u.data.length = sizeof(wirelessInterfaceParameters);

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
    fetchParameters();

    while (socketFileDescriptor) {
        int ioControlStatus;

        ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWSTATS, &statisticsRequest);
        if (ioControlStatus == -1) {
            std::cerr << "Error getting wireless interface statistics: " << strerror(errno) << "\n";
        }

        bool wasSignalLevelUpdated = static_cast<bool>(
                wirelessInterfaceStatistics.qual.updated & IW_QUAL_LEVEL_UPDATED
        );

        if (wasSignalLevelUpdated) {
            int signalLevel;

            if (powerDataUnit == IW_TXPOW_DBM) {
                signalLevel = wirelessInterfaceStatistics.qual.level - 0x100;
            } else {
                signalLevel = wirelessInterfaceStatistics.qual.level;
            }

            std::cout << "RX level: " << signalLevel << ((powerDataUnit == IW_TXPOW_DBM) ? " [dBm]" : "") << "\n";

            std::cout << 1 - calculateRXLevelRatio(signalLevel) << "%\n";
        }

        sleep(updateInterval);
    }
}

void WirelessSignalAdjuster::fetchParameters() {
    int ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWRANGE, &rangeRequest);
    if (ioControlStatus == -1) {
        std::cerr << "Error getting wireless interface range: " << strerror(errno) << "\n";
        std::exit(EXIT_FAILURE);
    }

    powerDataUnit = (wirelessInterfaceParameters.max_qual.updated & IW_QUAL_DBM) ? IW_TXPOW_DBM : IW_TXPOW_RANGE;

    if (powerDataUnit == IW_TXPOW_DBM) {
        minSignalLevel = wirelessInterfaceParameters.max_qual.level - 0x100;
        maxSignalLevel = 0;
    } else {
        minSignalLevel = 0;
        maxSignalLevel = wirelessInterfaceParameters.max_qual.level;
    }
    std::cout << "RX level in range [" << minSignalLevel << ":" << maxSignalLevel << "]";
    std::cout << ((powerDataUnit == IW_TXPOW_DBM) ? " [dBm]" : "") << "\n";

    maxSignalTXPowerNumber = wirelessInterfaceParameters.num_txpower;
    if (maxSignalTXPowerNumber > 0) {
        memcpy(maxSignalTXPower, wirelessInterfaceParameters.txpower, maxSignalTXPowerNumber);
        std::cout << "Maximum TX power\n";
    }
    for (int i = 0; i < maxSignalTXPowerNumber; i++) {
        std::cout << "Channel [" << i + 1 << "] " << wirelessInterfaceParameters.freq[i].e - 0x100 << " dBm\n";
    }
}

float WirelessSignalAdjuster::calculateRXLevelRatio(int level) {
    return (float)(level - minSignalLevel) / (float)(maxSignalLevel - minSignalLevel);
}
