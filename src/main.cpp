#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <setjmp.h>
#include <iostream>
// #include <3ds.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <stdbool.h>

#include "3ds.h"


#include "vision.h"

#define WAIT_TIMEOUT 1000000000ULL

#define WIDTH_TOP 400
#define HEIGHT_TOP 240
#define SCREEN_SIZE_TOP WIDTH_TOP * HEIGHT_TOP

#define WIDTH_BOTTOM 320
#define HEIGHT_BOTTOM  240
#define SCREEN_SIZE_BOTTOM  WIDTH_BOTTOM  * HEIGHT_BOTTOM 


static jmp_buf exitJmp;

inline void clearScreen(void) {
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(frame, 0, 320 * 240 * 3);
}

void cleanup() {
	camExit();
	gfxExit();
	acExit();
}

void hang(const char *message) {
	// clearScreen();
	printf("%s", message);
	// printf("Press start to exit");

    while (1)
    {
        hidScanInput();

        u32 kHeld = hidKeysHeld();
        if (kHeld & KEY_A) break;
    }

	// while (aptMainLoop()) {
	// 	hidScanInput();

	// 	u32 kHeld = hidKeysHeld();
	// 	if (kHeld & KEY_A) longjmp(exitJmp, 1);
	// }
}

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
    
    // OUTPUT_RGB_565 would compress 24bit RGB channels into 16bit
    // with trade off that less channels being represented
    // G have more channels because of human eyes are more sensitive to green
    CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A);
    CAMU_SetFrameRate(SELECT_OUT1_OUT2, FRAME_RATE_30);

    CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);

    CAMU_SetTrimming(PORT_CAM1, false);
    // CAMU_SetTrimming(PORT_CAM2, false); // I don't think we need second camera here

    u8 *buf = (u8 *) malloc(SCREEN_SIZE_TOP * 2);
	if(!buf) {
		hang("Failed to allocate memory!");
	}

    u32 bufSize;
    CAMU_GetMaxBytes(&bufSize, WIDTH_TOP, HEIGHT_TOP);
    CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH_TOP, HEIGHT_TOP);
    // CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH_TOP, HEIGHT_TOP);

    CAMU_Activate(SELECT_OUT1);
    // CAMU_Activate(SELECT_OUT1_OUT2);

    Handle camReceiveEvent[2] = {0};
    bool captureInterrupted = false;
    s32 index = 0;

    // We only need one camera to be functional
    CAMU_GetBufferErrorInterruptEvent(&camReceiveEvent[0], PORT_CAM1);
    // CAMU_GetBufferErrorInterruptEvent(&camReceiveEvent[1], PORT_CAM2);

    CAMU_ClearBuffer(PORT_CAM1);
    // CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);

    CAMU_StartCapture(PORT_CAM1);
    CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_MOVIE);

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    // // printf("\nUse slider to enable/disable 3D\n"); // Add we surely don't need 3D here
    // printf("Press Start to exit to Homebrew Launcher\n");

    // // Initialize services
    // gfxInitDefault();

    // //In this example we need one PrintConsole for each screen
    // PrintConsole topScreen, bottomScreen;

    // //Initialize console for both screen using the two different PrintConsole we have defined
    // consoleInit(GFX_TOP, &topScreen);
    // consoleInit(GFX_BOTTOM, &bottomScreen);

    // // Initialize top screen
    // consoleSelect(&topScreen);

    u32 kDown;
    u32 kHeld;

    std::cout << "Hello Nano" << std::endl;
    std::cout << std::to_string(bufSize) << std::endl;

    // Buffer allocating stuff


    // Loads of Camera initialization stuff


    
    while (aptMainLoop())
    {
        if (!captureInterrupted) 
        {
            // Read which buttons are currently pressed or not
            hidScanInput();
            kDown = hidKeysDown();

            // If START button is pressed, break loop and quit
            if (kDown & KEY_START)
            {
                break;
            }

            if (kDown & KEY_R)
            {
                hang("Capturing...\n");
            }
        }

        if (camReceiveEvent[1] == 0) 
        {
            // printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent[2], buf, PORT_CAM1, SCREEN_SIZE, (s16)bufSize));
            CAMU_SetReceiving(&camReceiveEvent[1], buf, PORT_CAM1, SCREEN_SIZE_TOP * 2, (s16)bufSize);
		}

        if (captureInterrupted) 
        {
            // printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_BOTH));
            CAMU_StartCapture(PORT_CAM1);
            captureInterrupted = false;
        }

        // Again we only tried to use one of them

        // if (camReceiveEvent[3] == 0) {
		// 	CAMU_SetReceiving(&camReceiveEvent[3], buf + SCREEN_SIZE, PORT_CAM2, SCREEN_SIZE, (s16)bufSize);
		// }
        svcWaitSynchronizationN(&index, camReceiveEvent, 2, false, WAIT_TIMEOUT);
        switch (index)
        {
            case 0:
                svcCloseHandle(camReceiveEvent[1]);
                camReceiveEvent[1] = 0;
                captureInterrupted = true;

                continue;
                break;

            case 1:
                svcCloseHandle(camReceiveEvent[1]);
                camReceiveEvent[1] = 0;

                break;

            default:
                break;

        }

        // svcWaitSynchronizationN(&index, camReceiveEvent, 4, false, WAIT_TIMEOUT);
        // switch (index) {
        // case 0:
        //     // printf("svcCloseHandle: 0x%08X\n", (unsigned int) svcCloseHandle(camReceiveEvent[2]));
        //     svcCloseHandle(camReceiveEvent[2]);
        //     camReceiveEvent[2] = 0;

        //     captureInterrupted = true;
        //     continue; //skip screen update
        //     break;
        // case 1:
        //     svcCloseHandle(camReceiveEvent[3]);
        //     camReceiveEvent[3] = 0;

        //     captureInterrupted = true;
        //     continue; //skip screen update
        //     break;
        // case 2:
        //     // printf("svcCloseHandle: 0x%08X\n", (unsigned int) svcCloseHandle(camReceiveEvent[2]));
        //     svcCloseHandle(camReceiveEvent[2]);
        //     camReceiveEvent[2] = 0;
        //     break;
        // case 3:
        //     svcCloseHandle(camReceiveEvent[3]);
        //     camReceiveEvent[3] = 0;
        //     break;
        //     default:
        //     break;
        // }

        // Decompress and display the image on top left screen 

        gfxSet3D(false);
        writePictureToFramebufferRGB565(
            gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 
            buf,
            0,
            0,
            WIDTH_TOP,
            HEIGHT_TOP
        );

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();


        // When pressed, Copy a frame into simpleocv
    
        // save image
    }

    // Stop camera, release allocated memory
    CAMU_StopCapture(PORT_CAM1);

    // Close camera event handles
    for (int i = 0; i < 4; i++)
    {
        if (camReceiveEvent[i] != 0) 
        {
            svcCloseHandle(camReceiveEvent[i]);
        }
    }

    // printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));
    CAMU_Activate(SELECT_NONE);

    // Exit
    free(buf);
    cleanup();
    return 0;

}