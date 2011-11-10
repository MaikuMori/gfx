#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <GL/glfw.h>
#include <GL/glext.h>

#include "common.h"
#include "hw1.h"
#include "hw3.h"

//Visual Leak Detector (http://vld.codeplex.com/)
//Uncomment to compile in Debug if you do not have Visual Leak Detector installed.
//I didn't include the lib in project settings so it's easier to disable it.
#if _DEBUG
//Found a bug in Visual Leak Detector which prevents to open windows dialogs
//while Visual Leak Detector is active. Disabling until the patch the problem.
//#pragma comment(lib,"C:\\Program Files (x86)\\Visual Leak Detector\\lib\\Win32\\vld.lib")
//#include "C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h"
#endif

int main(void)
{
    GLbyte running = GL_TRUE, hw_initialized = GL_FALSE;
    GLubyte hw_id;
    GLint i;

    void (* hw_init)(void) = NULL;
    void (* hw_draw)(void) = NULL;
    void (* hw_terminate)(void) = NULL;

    //Init GLFW
    if (!glfwInit()) {
        return EXIT_FAILURE;
    };

    //Init OpenGL context + window.
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    if (!glfwOpenWindow(TEXTURE_WIDTH, TEXTURE_HEIGHT, 8, 8, 8, 0, 0, 0, GLFW_WINDOW)) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwSetWindowPos(100, 100);
    MoveWindow(GetConsoleWindow(), 95, 100+TEXTURE_HEIGHT+35, 800, 400, TRUE);

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    //Init OpenGL.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    //Init textures.
    printf("Init textures ...\t");

    glGenTextures(TEXTURE_COUNT, &textures[0]);
    for (i = 0; i < TEXTURE_COUNT; i++) {
        bitmap_init(&texture_data[i]);
    }

    printf("DONE!\n");

    //Start off with homework 1.
    hw_id = 1;
    hw_init = hw1_init;
    hw_draw = hw1_draw;
    hw_terminate = hw1_terminate;

    printf("Entering main loop:\n");

    while(running) {
        glClear(GL_COLOR_BUFFER_BIT);

        //Initialize homework.
        if (!hw_initialized) {
            hw_init();
            hw_initialized = GL_TRUE;
        }

        //Call the current homework draw function.
        hw_draw();

        glfwSwapBuffers();

        //Crude input handler, you can select which homework to display by using keys 1-9.
        if (glfwGetWindowParam(GLFW_OPENED)) {
            running = !glfwGetKey(GLFW_KEY_ESC);

            if (glfwGetKey('1') && hw_id != 1) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 1;
                hw_init = hw1_init;
                hw_draw = hw1_draw;
                hw_terminate = hw1_terminate;
            }

            if (glfwGetKey('3') && hw_id != 3) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 3;
                hw_init = hw3_init;
                hw_draw = hw3_draw;
                hw_terminate = hw3_terminate;
            }

        } else {
            running = GL_FALSE;
        }
    }

    printf("Main loop terminated, freeing stuff ...\t");

    //Terminate the current homework.
    if (hw_initialized) {
        hw_terminate();
    }

    //Free the textures.
    for (i = 0; i < TEXTURE_COUNT; i++) {
        bitmap_free(&texture_data[i]);
    }
    glDeleteTextures(TEXTURE_COUNT, &textures[0]);

    printf("DONE!\n");

    printf("Terminating OpenGL context ...\t");
    glfwTerminate();
    printf("DONE!\nBye!\n");

    return EXIT_SUCCESS;
}