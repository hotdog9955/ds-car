// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <nds.h>
#include <dswifi9.h>


// dumps recv values
void dumpSockOutput(int s)
{
    char *junkbuf[128];
    recv(s, junkbuf, sizeof(junkbuf), 0);
}

int initOBD()
{

    // init wifi with default settings
    if (!Wifi_InitDefault(WFC_CONNECT))
    {
        printf("Can't connect to WiFi!\n");
        return -1;
    }

    // create tcp socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf("failed to create socket.");
        return -1;
    }

    // Tell the socket to connect to the IP address of the OBD reader
    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons(35000);
    // this is the ip 192.168.0.10, its in this format u8.u8.u8.u8, it is reversed too, C0 is 192.
    sain.sin_addr.s_addr = 0x0A00A8C0UL;

    // print out the ip
    unsigned long ip = 0x0A00A8C0UL;
    printf("IP Address: %ld.%ld.%ld.%ld\n", (ip >> 0) & 0xFF, (ip >> 8) & 0xFF,
           (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);

    if (connect(s, (struct sockaddr *)&sain, sizeof(sain)) == -1)
    {
        printf("failed to connect socket.");
        close(s);
        return -1;
    }

    // reset the OBD reader
    send(s, "ATZ\r", 5, 0);

    // wait for 3 secs for it to reset
    for (size_t i = 0; i < 60 * 3; i++)
    {
        swiWaitForVBlank();
    }

    // buffer for junk, output to this should not be used
    dumpSockOutput(s);

    // set elm327 to default settings
    send(s, "ATD\r", 5, 0);
    dumpSockOutput(s);

    send(s, "ATE0\r", 6, 0);
    dumpSockOutput(s);

    send(s, "ATAL\r", 6, 0);
    dumpSockOutput(s);

    send(s, "ATSP0\r", 7, 0);
    dumpSockOutput(s);

    send(s, "0100\r", 6, 0);
    dumpSockOutput(s);

    return s;
}

int main(int argc, char *argv[])
{
    consoleDemoInit();

    printf("Connecting to WiFi using\n");
    printf("firmware settings...\n");

    int s = initOBD();

    if (s == -1)
    {
        printf("failed to connect to OBD");
        return -1;
    }

    printf("\n");
    printf("Press START to exit");

    while (1)
    {
        swiWaitForVBlank();
        scanKeys();
        if (keysHeld() & KEY_START)
            break;
    }

    return 0;
}
