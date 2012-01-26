#include <stdio.h>
#include <stdlib.h>

#include <GL/glfw.h>

#include "hw6.h"
#include "common.h"

#define HW6_BORDER (TEXTURE_WIDTH - 360) / 2
static void draw_rainbow(void)
{
    int y;
    Color c;

    //Color the texture light blue.
    bitmap_fill(&texture_data[3], 0, 191, 255);

    //Draw the rainbow using HSV. Could do something fancy here but got no
    //time left.
    for (y = HW6_BORDER; y < 360 + HW6_BORDER; y++) {
        hsv2rgb(y - HW6_BORDER, 100, 100, &c);
        bitmap_draw_line(&texture_data[3], 0, y, TEXTURE_WIDTH - 1, y,
            c.red, c.green, c.blue);
    }

    bitmap_upload(&texture_data[3],textures[3]);
}

static void print_help(void)
{
    printf("\nHelp:\n");
    printf(" Nothing special here just pretty pictures.\n");
}

void hw6_init(void)
{
    printf("\nInitializing homework 6 ...\n");

    glfwSetWindowTitle("GFX Homework: 6.2a");


    //Setup OpenGL.

    //Change the window size and OpenGL viewport.
    glfwSetWindowSize(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    //Reset projection matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //Set up orthographic projection.
    glOrtho(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, -1, 1);
    //Reset model view matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //Enable textures globally since we're just drawing textured quad.
    glEnable(GL_TEXTURE_2D);

    //Yeeeey!
    draw_rainbow();

    printf("DONE!\n");

    print_help();
}

void hw6_draw(void)
{
    //Draw quad and texture it using our bitmap.
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2i(0, 1);
    glVertex3f(0, TEXTURE_HEIGHT, 0);
    glTexCoord2i(1, 1);
    glVertex3f(TEXTURE_WIDTH, TEXTURE_HEIGHT, 0);
    glTexCoord2i(1, 0);
    glVertex3f(TEXTURE_WIDTH, 0, 0);
    glEnd();
}

void hw6_terminate(void)
{
    //Restore OpenGL settings.
    glDisable(GL_TEXTURE_2D);
}