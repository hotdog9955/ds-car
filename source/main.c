#include <stdio.h>
#include <nds.h>
#include "obd.h"

int main(int argc, char *argv[])
{
    // Set up the video modes for the top and bottom screens
    videoSetMode(MODE_0_2D);      // Main screen (top screen) to 2D mode
    videoSetModeSub(MODE_0_2D);   // Sub screen (bottom screen) to 2D mode

    // Configure VRAM for background graphics
    vramSetBankA(VRAM_A_MAIN_BG); // Use VRAM bank A for main background
    vramSetBankC(VRAM_C_SUB_BG);  // Use VRAM bank C for sub background

    // Initialize consoles for both screens
    PrintConsole topScreenConsole, bottomScreenConsole;

    // Initialize the console on the main screen (top screen)
    consoleInit(
        &topScreenConsole,       // Pointer to the console structure
        0,                       // Background layer
        BgType_Text4bpp,         // Background type
        BgSize_T_256x256,        // Background size
        31,                      // Map base (BG_MAP_RAM(31))
        0,                       // Tile base (BG_TILE_RAM(0))
        true,                    // Main screen (set to true for the top screen)
        true                     // Load console graphics
    );

    // Initialize the console on the sub screen (bottom screen)
    consoleInit(
        &bottomScreenConsole,
        0,
        BgType_Text4bpp,
        BgSize_T_256x256,
        31,
        0,
        false,                   // Sub screen (set to false for the bottom screen)
        true
    );

    // Select the top screen console and initialize it as the debug console
    consoleSelect(&topScreenConsole);
    consoleDebugInit(DebugDevice_CONSOLE);

    consoleSelect(&bottomScreenConsole);

    fprintf(stderr, "Error printing! yay!\n");

    printf("Connecting to WiFi using\n");
    printf("firmware settings...\n");

    OBDSocket s = initConnection();

    if(s == -1){
        fprintf(stderr, "failed to connect.");
        return -1;
    }


    if(initELM(s) < 0){
        fprintf(stderr, "failed to init ELM327");
        return -1;
    };

    printf("testing obd!\n");
    testELM(s, OBD_BUF);

    printf("FINISHED!\n");
    printf("looping now...\n");
    while (1)
    {
        for (size_t i = 0; i < 60; i++)
        {
            swiWaitForVBlank();
        }

        testELM(s, OBD_BUF);
        
    }
    

    return 0;
}
