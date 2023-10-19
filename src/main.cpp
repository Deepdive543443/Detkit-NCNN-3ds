#include <setjmp.h>
#include <iostream> // std

#include "3ds.h" // 3ds devkit 

#include "net.h"
#include "simpleocv.h" // ncnn

#include "vision.h" 
#include "nanodet.h"// utils

#define WAIT_TIMEOUT 1000000000ULL

#define WIDTH_TOP 400
#define HEIGHT_TOP 240
#define SCREEN_SIZE_TOP WIDTH_TOP * HEIGHT_TOP

#define WIDTH_BOTTOM 320
#define HEIGHT_BOTTOM  240
#define SCREEN_SIZE_BOTTOM  WIDTH_BOTTOM  * HEIGHT_BOTTOM 

static jmp_buf exitJmp;

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

void hang(const char *message, void* buf, Nanodet &nanodet) 
{
	// clearScreen();
	printf("%s", message);
	// printf("Press start to exit");

    while (1)
    {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_B) break;

        if (kDown & KEY_A)
        {
            // ncnn::Mat image(WIDTH_TOP, HEIGHT_TOP, 3);
            cv::Mat image_ocv(320, 320, 3);
            
            {
                ncnn::Mat image(WIDTH_TOP, HEIGHT_TOP, 3);
                ncnn::Mat resized(320, 320, 3);

                writePictureToMat(image, buf, 0, 0, WIDTH_TOP, HEIGHT_TOP);
                printf("H: %d, W: %d, C: %d\n", image.h, image.w, image.c);

                bordered_resize(image, resized, 320);

                // cv::Mat image_ocv(image.h * 2, image.w * 2, 3);
                // image.to_pixels_resize(image_ocv.data, ncnn::Mat::PIXEL_BGR, image.w * 2, image.h * 2);
                // ncnn::resize_bilinear(
                //    image, 
                //    resized, 
                //    320, 
                //    320
                // );
                // ncnn::resize_bilinear_c3(
                //     (unsigned char *) image.data,
                //     WIDTH_TOP, 
                //     WIDTH_TOP, 
                //     (unsigned char *) resized.data, 
                //     320, 
                //     320
                // );

                resized.to_pixels(image_ocv.data, ncnn::Mat::PIXEL_BGR);
            }
            

            cv::imwrite("test_image/test_img_resized.png", image_ocv);
            printf("Save image resized\n");
            printf("Yes  w: %d, h: %d\n", image_ocv.cols, image_ocv.rows);
        }

        if (kDown & KEY_X)
        {
            // ncnn::Mat image(WIDTH_TOP, HEIGHT_TOP, 3); // should be (w, h, c)
            ncnn::Mat image(WIDTH_TOP, WIDTH_TOP, 3);
            cv::Mat image_ocv(WIDTH_TOP, WIDTH_TOP, 3);
            
            writePictureToMat(image, buf, 0, HEIGHT_TOP / 4, WIDTH_TOP, HEIGHT_TOP);
            printf("H: %d, W: %d, C: %d\n", image.h, image.w, image.c);
            
            // cv::Mat image_ocv(HEIGHT_TOP, WIDTH_TOP, 3);
            image.to_pixels(image_ocv.data, ncnn::Mat::PIXEL_BGR);

            // cv::imwrite("sdmc:/image/test_img.png", image_ocv);
            cv::imwrite("test_image/test_img.png", image_ocv);

            printf("H: %d, W: %d, C: %d\n", image.h, image.w, image.c);
            printf("Save image\n");
        }

        if(kDown & KEY_Y)
        {
            // ncnn::Mat image(WIDTH_TOP, HEIGHT_TOP, 3);
            // writePictureToMat(image, buf, 0, 0, WIDTH_TOP, HEIGHT_TOP);
            ncnn::Mat image(WIDTH_TOP, WIDTH_TOP, 3);
            writePictureToMat(image, buf, 0, HEIGHT_TOP / 4, WIDTH_TOP, HEIGHT_TOP);
            nanodet.forward_test(image);
        }
    }
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
    // u32 kHeld;

    // Initialize nanodet
    ncnn::Option opt;
    opt.lightmode = true;
    opt.num_threads = 1;
    opt.use_winograd_convolution = true;
    opt.use_sgemm_convolution = true;
    opt.use_int8_inference = true;
    opt.use_vulkan_compute = false;
    opt.use_fp16_packed = true;
    opt.use_fp16_storage = true;
    opt.use_fp16_arithmetic = true;
    opt.use_int8_storage = true;
    opt.use_int8_arithmetic = true;
    opt.use_packing_layout = true;
    opt.use_shader_pack8 = false;
    opt.use_image_storage = false;

    Nanodet nanodet;

    // Rom file system pattern
    // Result rc = romfsInit();
    // if (rc)
    //     printf("romfsInit: %08lX\n", rc);

    // else
    // {
    //     nanodet.create("romfs:/models/nanodet-m.param", "romfs:/models/nanodet-m.bin", opt);
    //     printf("romfs Init Successful!\n");
    // }

    if (nanodet.create("models/nanodet-plus-m_416.param", "models/nanodet-plus-m_416.bin", opt))
    {
        hang_err("Failed loading nanodet");
    }

    std::cout << "Hello Nano" << std::endl;
    std::cout << std::to_string(bufSize) << std::endl;
    
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
                hang("Capturing...\n", buf, nanodet);
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

    // printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));
    CAMU_Activate(SELECT_NONE);

    // Exit
    free(buf);
    cleanup();
    return 0;

}