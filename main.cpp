#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/wireless.h>

#include <stdlib.h>

/* The name of the interface */
#ifndef IW_NAME
#define IW_NAME "wlp3s0f0"
#endif


int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}