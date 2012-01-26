#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/glext.h>

#include "common.h"

#include "hw1.h"
#include "hw2.h"
#include "hw3.h"
#include "hw4.h"

#include "hw6.h"
#include "hw8.h"
#include "hw9.h"

//Visual Leak Detector (http://vld.codeplex.com/)
//Uncomment to compile in Debug if you do not have Visual Leak Detector installed.
//I didn't include the lib in project settings so it's easier to disable it.
#if _DEBUG
//Found a bug in Visual Leak Detector which prevents to open windows dialogs
//while Visual Leak Detector is active. Disabling until the patch the problem.
//#pragma comment(lib,"C:\\Program Files (x86)\\Visual Leak Detector\\lib\\Win32\\vld.lib")
//#include "C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h"
#endif

static void print_help(void)
{
    printf("\nHelp:\n");
    printf(" Use keys to [1-4, 6, 8-9] to switch between homeworks.\n");
    printf("------\n");
}

int main(void)
{
    GLbyte running = GL_TRUE, hw_initialized = GL_FALSE;
    GLubyte hw_id;
    GLint i;

    void (* hw_init)(void) = NULL;
    void (* hw_draw)(void) = NULL;
    void (* hw_terminate)(void) = NULL;

    //Initialize GLFW.
    if (!glfwInit()) {
        error("Failed to initialize GLFW.");
        return EXIT_FAILURE;
    };

    //Initialize OpenGL context + window.
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    if (!glfwOpenWindow(TEXTURE_WIDTH, TEXTURE_HEIGHT, 8, 8, 8, 8, 8, 8, GLFW_WINDOW)) {
        glfwTerminate();
        error("Failed to initialize OpenGL context.");
        return EXIT_FAILURE;
    }

    //Initialize GLEW. (Helps checking for and loading OpenGL extensions).
    if (glewInit() != GLEW_OK) {
        error("Failed to initialize GLEW.");
        return EXIT_FAILURE;
    }

    //Set the initial window position.
    glfwSetWindowPos(100, 100);
    MoveWindow(GetConsoleWindow(), 95, 100+TEXTURE_HEIGHT+35, 800, 400, TRUE);

    //Print OpenGL driver version.
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    //Initialize textures.
    glGenTextures(TEXTURE_COUNT, &textures[0]);
    for (i = 0; i < TEXTURE_COUNT; i++) {
        bitmap_init(&texture_data[i]);
    }

    //Start off with homework 1.
    hw_id = 1;
    hw_init = hw1_init;
    hw_draw = hw1_draw;
    hw_terminate = hw1_terminate;

    print_help();

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
            } else if (glfwGetKey('2') && hw_id != 2) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 2;
                hw_init = hw2_init;
                hw_draw = hw2_draw;
                hw_terminate = hw2_terminate;
            } else if (glfwGetKey('3') && hw_id != 3) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 3;
                hw_init = hw3_init;
                hw_draw = hw3_draw;
                hw_terminate = hw3_terminate;
            } else if (glfwGetKey('4') && hw_id != 4) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 4;
                hw_init = hw4_init;
                hw_draw = hw4_draw;
                hw_terminate = hw4_terminate;
            } else if (glfwGetKey('6') && hw_id != 6) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 6;
                hw_init = hw6_init;
                hw_draw = hw6_draw;
                hw_terminate = hw6_terminate;
            } else if (glfwGetKey('8') && hw_id != 8) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 8;
                hw_init = hw8_init;
                hw_draw = hw8_draw;
                hw_terminate = hw8_terminate;
            } else if (glfwGetKey('9') && hw_id != 9) {
                if (hw_initialized) {
                    hw_terminate();
                    hw_initialized = GL_FALSE;
                }

                hw_id = 9;
                hw_init = hw9_init;
                hw_draw = hw9_draw;
                hw_terminate = hw9_terminate;
            }
        } else {
            running = GL_FALSE;
        }
    }

    //Terminate the current homework.
    if (hw_initialized) {
        hw_terminate();
    }

    //Free the textures.
    for (i = 0; i < TEXTURE_COUNT; i++) {
        bitmap_free(&texture_data[i]);
    }
    glDeleteTextures(TEXTURE_COUNT, &textures[0]);

    //Terminate OpenGL context.
    glfwTerminate();

    return EXIT_SUCCESS;
}