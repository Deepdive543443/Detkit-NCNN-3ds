#include <3ds.h>

#include <iostream>

#define WIDTH 400
#define HEIGHT 240

int main(int argc, char** argv)
{
    // Lof of Camera initial stuff copy from video example
    // Initializations
    acInit();
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    // Enable double buffering to remove screen tearing
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    // Save current stack frame for easy exit
    if(setjmp(exitJmp)) {
        cleanup();
        return 0;
    }

    camInit();
    CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A);
    CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A);












    // Initialize services
    gfxInitDefault();

    //In this example we need one PrintConsole for each screen
    PrintConsole topScreen, bottomScreen;

    //Initialize console for both screen using the two different PrintConsole we have defined
    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);

    // Initialize top screen
    consoleSelect(&topScreen);

    u32 kDown;
    u32 kHeld;

    std::cout << "Hello Nano" << std::endl;

    // Buffer allocating stuff


    // Loads of Camera initialization stuff


    
    while (aptMainLoop())
    {
        hidScanInput();
        kDown = hidKeysDown();
        kHeld = hidKeysHeld();

        // Quit App
        if(kHeld & KEY_START) break;

        // When pressed, Copy a frame into simpleocv
    
        // save image
    }
    

}