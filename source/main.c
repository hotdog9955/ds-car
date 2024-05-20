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


    int rpm = getRPM(s);
    
    printf("engine RPM: %d\n", rpm);

    while (1)
    {
        printf("%d\n",getRPM(s));
    }

    return 0;
}
