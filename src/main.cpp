#include <iostream>
#include <chrono>  // std
#include <setjmp.h>
#include "3ds.h"  // 3ds devkit
#include "net.h"
#include "simpleocv.h"  // ncnn

#include "detector.h"
#include "vision.h"

#define WAIT_TIMEOUT    1000000000ULL
#define WIDTH_TOP       400
#define HEIGHT_TOP      240
#define SCREEN_SIZE_TOP WIDTH_TOP *HEIGHT_TOP

static jmp_buf                     exitJmp;
static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator         g_workspace_pool_allocator;

static void cleanup()
{
    camExit();
    gfxExit();
    acExit();
}

static void hang_err(const char *message)
{
    printf("%s\nPress START to exit\n", message);
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;
    }
}

static double get_current_time()
{
    auto now  = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return usec.count() / 1000.0;
}

int main(int argc, char **argv)
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
    if (setjmp(exitJmp)) {
        cleanup();
        return 0;
    }

    camInit();
    CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A);
    CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);
    CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30);
    CAMU_SetNoiseFilter(SELECT_OUT1, true);
    CAMU_SetAutoExposure(SELECT_OUT1, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);
    CAMU_SetTrimming(PORT_CAM1, false);

    void *buf = malloc(SCREEN_SIZE_TOP * 2);  // RBG565 frame buffer
    if (!buf) {
        hang_err("Failed to allocate memory!");
    }

    u32 bufSize;
    CAMU_GetMaxBytes(&bufSize, WIDTH_TOP, HEIGHT_TOP);
    CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH_TOP, HEIGHT_TOP);
    // CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH_TOP, HEIGHT_TOP);

    CAMU_Activate(SELECT_OUT1);

    Handle camReceiveEvent[2] = {0};
    bool   captureInterrupted = false;
    s32    index              = 0;

    CAMU_GetBufferErrorInterruptEvent(&camReceiveEvent[0], PORT_CAM1);
    CAMU_ClearBuffer(PORT_CAM1);
    CAMU_StartCapture(PORT_CAM1);
    CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_MOVIE);

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    // Button place holder
    u32 kDown;

    // Rom file system pattern
    Result rc = romfsInit();
    if (rc)
        printf("romfsInit: %08lX\n", rc);

    else {
        printf("romfs Init Successful!\n");
    }

    Detector    *dtr_ptr      = create_detector(NANODET_PLUS, "romfs:/config/nanodet-plus-m_416_int8.json");
    Detector_idx detector_idx = NANODET_PLUS;

    printf("Hello Nano\nPress R to detect\nPress L to switch to FastestDet\n");
    while (aptMainLoop()) {
        if (camReceiveEvent[1] == 0) {
            CAMU_SetReceiving(&camReceiveEvent[1], buf, PORT_CAM1, SCREEN_SIZE_TOP * 2, (s16)bufSize);
        }

        if (captureInterrupted) {
            CAMU_StartCapture(PORT_CAM1);
            captureInterrupted = false;
        }

        svcWaitSynchronizationN(&index, camReceiveEvent, 2, false, WAIT_TIMEOUT);
        switch (index) {
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
        writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0, WIDTH_TOP,
                                        HEIGHT_TOP);

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();

        if (!captureInterrupted) {
            // Read which buttons are currently pressed or not
            hidScanInput();
            kDown = hidKeysDown();

            // If START button is pressed, break loop and quit
            if (kDown & KEY_START) break;

            if (kDown & KEY_L) {
                dtr_ptr->clear();
                delete dtr_ptr;
                dtr_ptr = 0;

                switch (detector_idx) {
                    case NANODET_PLUS:
                        dtr_ptr      = create_detector(FASTESTDET, "romfs:/config/fastestdet.json");
                        detector_idx = FASTESTDET;

                        printf("\nLoad Fastest Det successful\nPress L to switch to Nanodet\n");
                        break;

                    case FASTESTDET:
                        dtr_ptr      = create_detector(NANODET_PLUS, "romfs:/config/nanodet-plus-m_416_int8.json");
                        detector_idx = NANODET_PLUS;
                        printf("\nLoad Nanodet successful\nPress L to switch to FastestDet\n");
                        break;

                    default:
                        break;
                }
            }

            if (kDown & KEY_X) {
                printf("\nInference testing\n");
                double start = get_current_time();
                dtr_ptr->inference_test();
                double end  = get_current_time();
                double time = end - start;
                printf("Time: %7.2f\n", time);
            }

            if (kDown & KEY_R) {
                g_blob_pool_allocator.clear();
                g_workspace_pool_allocator.clear();

                cv::Mat image_output(HEIGHT_TOP, WIDTH_TOP, 3);
                writePictureToMat(image_output, buf, 0, 0, WIDTH_TOP, HEIGHT_TOP);
                std::vector<BoxInfo> bboxes;

                {
                    printf("\nDetecting\n");
                    double start = get_current_time();
                    bboxes       = dtr_ptr->detect(image_output);
                    double end   = get_current_time();
                    double time  = end - start;
                    printf("Time: %7.2f\n", time);
                }

                dtr_ptr->draw_boxxes(image_output, bboxes);
                writeMatToFrameBuf(image_output, gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0, 0, WIDTH_TOP,
                                   HEIGHT_TOP);
                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();

                printf("Press B to continue\n");
                while (1) {
                    hidScanInput();
                    u32 kDown = hidKeysDown();
                    if (kDown & KEY_B) break;
                }
            }
        }
    }

    // Stop camera, release allocated memory
    dtr_ptr->clear();
    CAMU_StopCapture(PORT_CAM1);

    // Close camera event handles
    for (int i = 0; i < 2; i++) {
        if (camReceiveEvent[i] != 0) {
            svcCloseHandle(camReceiveEvent[i]);
        }
    }
    CAMU_Activate(SELECT_NONE);

    // Exit
    free(buf);
    cleanup();
    return 0;
}