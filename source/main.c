#include <stdio.h>
#include <nds.h>
#include "obd.h"

int main(int argc, char *argv[])
{
    consoleDemoInit();

    printf("Connecting to WiFi using\n");
    printf("firmware settings...\n");

    OBDSocket s = initOBD();

    if (s == -1)
    {
        printf("failed to connect to ELM327");
        return -1;
    }


    printf("engine RPM: %d", getRPM(s));
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
