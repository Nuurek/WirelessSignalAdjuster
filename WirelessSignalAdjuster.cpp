//
// Created by nuurek on 11.06.18.
//

#include <cmath>
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

    strncpy(txPowerRequest.ifr_name, wirelessInterfaceName, IFNAMSIZ);
    strncpy(channelRequest.ifr_name, wirelessInterfaceName, IFNAMSIZ);

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
    getSignalParameters();

    while (socketFileDescriptor) {
        bool wasSignalLevelUpdated = getSignalStatistics();

        if (wasSignalLevelUpdated) {
            int rxLevel = getRXLevel();
            float rxLevelRatio = calculateRXLevelRatio(rxLevel);

            std::cout << "RX level: " << rxLevel << ((powerDataUnit == IW_TXPOW_DBM) ? " [dBm]" : "");
            std::cout << " (" << rxLevelRatio * 100 << "%)\n";


            float txLevelRatio = 1.0f - rxLevelRatio;
            int desiredTXLevel;
            if (maxTXPowerNumber > 0) {
                int channel = getChannel();
                std::cout << "Channel: " << channel << "\n";
                desiredTXLevel = static_cast<int>(txLevelRatio * (float) maxTXPower[channel]);
            } else {
                desiredTXLevel = static_cast<int>(txLevelRatio * (float) defaultTXPower);
            }

            std::cout << "TX level: " << desiredTXLevel << " [dBm]";
            std::cout << " (" << txLevelRatio * 100 << "%)\n";

            setTXLevel(desiredTXLevel);
        }

        sleep(updateInterval);
    }
}

void WirelessSignalAdjuster::getSignalParameters() {
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

    maxTXPowerNumber = wirelessInterfaceParameters.num_txpower;
    if (maxTXPowerNumber > 0) {
        memcpy(maxTXPower, wirelessInterfaceParameters.txpower, maxTXPowerNumber);
        std::cout << "Maximum TX power\n";
    } else {
        defaultTXPower = getTXLevel();
        std::cout << "Default TX power: " << defaultTXPower << " [dBm]\n";
    }
    for (int i = 0; i < maxTXPowerNumber; i++) {
        std::cout << "Channel [" << i + 1 << "] " << wirelessInterfaceParameters.freq[i].e - 0x100 << " dBm\n";
    }
}

bool WirelessSignalAdjuster::getSignalStatistics() {
    int ioControlStatus;

    ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWSTATS, &statisticsRequest);
    if (ioControlStatus == -1) {
        std::cerr << "Error getting wireless interface statistics: " << strerror(errno) << "\n";
    }

    return static_cast<bool>(
            wirelessInterfaceStatistics.qual.updated & IW_QUAL_LEVEL_UPDATED
    );
}

int WirelessSignalAdjuster::getRXLevel() {
    int rxLevel;

    if (powerDataUnit == IW_TXPOW_DBM) {
        rxLevel = wirelessInterfaceStatistics.qual.level - 0x100;
    } else {
        rxLevel = wirelessInterfaceStatistics.qual.level;
    }

    return rxLevel;
}

float WirelessSignalAdjuster::calculateRXLevelRatio(int level) {
    return (float)(level - minSignalLevel) / (float)(maxSignalLevel - minSignalLevel);
}

int WirelessSignalAdjuster::getTXLevel() {
    txPowerRequest.u.txpower.value = -1;
    txPowerRequest.u.txpower.fixed = 1;
    txPowerRequest.u.txpower.disabled = 0;
    txPowerRequest.u.txpower.flags = static_cast<__u16 >(powerDataUnit);

    int ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWTXPOW, &txPowerRequest);
    if (ioControlStatus == -1) {
        std::cerr << "Error getting TX power: " << strerror(errno) << "\n";
    }

    return txPowerRequest.u.txpower.value;
}

int WirelessSignalAdjuster::getChannel() {
    int ioControlStatus = ioctl(socketFileDescriptor, SIOCGIWFREQ, &rangeRequest);
    if (ioControlStatus == -1) {
        std::cerr << "Error getting TX power: " << strerror(errno) << "\n";
    }

    double frequency, referenceFrequency;
    int channel = -1;

    frequency = ((double) rangeRequest.u.freq.m) * pow(10, rangeRequest.u.freq.e);
    for (int i = 0; i < wirelessInterfaceParameters.num_frequency; i++) {
        iw_freq freq = wirelessInterfaceParameters.freq[i];
        referenceFrequency = ((double) freq.m) * pow(10, freq.e);
        if (referenceFrequency == frequency) {
            channel = freq.i;
            break;
        }
    }

    return channel;
}

void WirelessSignalAdjuster::setTXLevel(int level) {
    txPowerRequest.u.txpower.value = level;
    txPowerRequest.u.txpower.fixed = 1;
    txPowerRequest.u.txpower.disabled = 0;
    txPowerRequest.u.txpower.flags = IW_TXPOW_DBM;

    int ioControlStatus = ioctl(socketFileDescriptor, SIOCSIWTXPOW, &txPowerRequest);
    if (ioControlStatus == -1) {
        std::cerr << "Error setting TX power: " << strerror(errno) << "\n";
    }
}
