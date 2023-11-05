#include <setjmp.h>
#include <iostream> 
#include <chrono> // std

#include "3ds.h" // 3ds devkit 

#include "net.h"
#include "simpleocv.h" // ncnn

#include "vision.h" 
#include "nanodet.h"// utils

#define WAIT_TIMEOUT 1000000000ULL
#define WIDTH_TOP 400
#define HEIGHT_TOP 240
#define SCREEN_SIZE_TOP WIDTH_TOP * HEIGHT_TOP

static jmp_buf exitJmp;
static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

void cleanup() 
{
    camExit();
    gfxExit();
    acExit();
}

void hang_err(const char *message)
{
    printf("%s\nPress START to exit\n", message);
    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;
    }
}

double get_current_time()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return usec.count() / 1000.0;
}

int main(int argc, char** argv)
{
    // Loads of Camera initial stuff copy from video example
    // Initializations
    acInit();
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    // Enable double buffering to remove screen tearing
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    // Save current stack frame for easy exit
    if(setjmp(exitJmp)) 
    {
        cleanup();
        return 0;
    }

    camInit();
    CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A);
    
    CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A);
    CAMU_SetFrameRate(SELECT_OUT1_OUT2, FRAME_RATE_30);

    CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);

    CAMU_SetTrimming(PORT_CAM1, false);
    // CAMU_SetTrimming(PORT_CAM2, false); // I don't think we need second camera here

    void *buf = malloc(SCREEN_SIZE_TOP * 2);
    if(!buf)
    {
        hang_err("Failed to allocate memory!");
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


    u32 kDown;

    Nanodet nanodet;
    // Rom file system pattern
    Result rc = romfsInit();
    if (rc)
        printf("romfsInit: %08lX\n", rc);

    else
    {
        nanodet.load_param("romfs:/config/nanodet-plus-m_416-int8.json");
        printf("romfs Init Successful!\n");
    }

    int inference_size = nanodet.input_size[0];
    int drawing_coor = ((float) inference_size / 2 ) - (inference_size / 2);
    float scale = (float) WIDTH_TOP / inference_size;

    printf("Hello Nano\nPress R to detect\n");
    
    while (aptMainLoop())
    {
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

        if (!captureInterrupted) 
        {
            // Read which buttons are currently pressed or not
            hidScanInput();
            kDown = hidKeysDown();

            // If START button is pressed, break loop and quit
            if (kDown & KEY_START) break;
            
            if (kDown & KEY_X)
            {
                printf("\nInference testing\n");

                double start = get_current_time();
                nanodet.inference_test();
                double end = get_current_time();

                double time = end - start;
                printf("Time: %7.2f\n", time);
            }

            if (kDown & KEY_R)
            {
                g_blob_pool_allocator.clear();
                g_workspace_pool_allocator.clear();

                cv::Mat image_output(HEIGHT_TOP, WIDTH_TOP,  3);
                std::vector<BoxInfo> bboxes;

                {
                    ncnn::Mat image(WIDTH_TOP, HEIGHT_TOP, 3);
                    writePictureToMat(image, buf, 0, 0, WIDTH_TOP, HEIGHT_TOP);
                    image.to_pixels(image_output.data, ncnn::Mat::PIXEL_BGR);

                    printf("\nDetecting\n");
                    double start = get_current_time();
                    bboxes = nanodet.detect(image);
                    double end = get_current_time();

                    double time = end - start;
                    printf("Time: %7.2f\n", time);
                }

                draw_bboxes(image_output, bboxes, drawing_coor, scale);

                writeMatToFrameBuf(image_output, gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0, 0, WIDTH_TOP, HEIGHT_TOP);
                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();

                printf("Press B to continue\n");
                while(1)
                {
                    hidScanInput();
                    u32 kDown = hidKeysDown();
                    if (kDown & KEY_B) break;
                }
            }
        }
    }

    // Stop camera, release allocated memory
    CAMU_StopCapture(PORT_CAM1);

    // Close camera event handles
    for (int i = 0; i < 2; i++)
    {
        if (camReceiveEvent[i] != 0) 
        {
            svcCloseHandle(camReceiveEvent[i]);
        }
    }
    CAMU_Activate(SELECT_NONE);

    // Exit
    free(buf);
    cleanup();
    return 0;
}