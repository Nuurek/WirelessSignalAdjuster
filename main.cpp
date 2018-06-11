#include <iostream>
#include <sys/socket.h>
#include <linux/wireless.h>

#include <cstring>
#include <zconf.h>
#include <sys/ioctl.h>

#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif


int main() {
    int socketFileDescriptor;
    struct iw_statistics wirelessInterfaceStatistics;

    struct iwreq request;
    strcpy(request.ifr_name, IW_NAME);
    request.u.data.pointer = &wirelessInterfaceStatistics;
    request.u.data.flags = 1;
    request.u.data.length = sizeof(wirelessInterfaceStatistics);

    socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFileDescriptor == -1) {
        std::cerr << "Error creating datagram socket: " << strerror(errno) << "\n";
        std::exit(EXIT_FAILURE);
    }

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

        sleep(1);
    }

    close(socketFileDescriptor);

    return 0;
}