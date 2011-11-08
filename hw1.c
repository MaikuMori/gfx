#include <stdio.h>
#include <stdlib.h>

#include <GL/glfw.h>

#include "hw1.h"
#include "common.h"

#define BORDER 5

void hw1_init()
{
	GLubyte c = 0;

	printf("Initializing homework 1 ...\t");

	glfwSetWindowTitle("GFX Homework: 1.a");

	//Color them white.
	bitmap_fill(&texture_data[1], 255, 255, 255);
	bitmap_fill(&texture_data[2], 255, 255, 255);
	bitmap_upload(&texture_data[1], textures[1]);
	bitmap_upload(&texture_data[2], textures[2]);

	//Change the window size and OpenGL viewport.
	glfwSetWindowSize(TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT);
	glViewport(0, 0, TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT, 0, -1, 1);

	printf("DONE!\n");
}

void hw1_draw()
{
	//Draw quads and texture them using our bitmaps.
	glEnable(GL_TEXTURE_2D);

	//The picture.
	glBindTexture(GL_TEXTURE_2D, textures[1]);
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

	//The histogram.
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex3f(TEXTURE_WIDTH + BORDER, 0, 0);
	glTexCoord2i(0, 1);
	glVertex3f(TEXTURE_WIDTH + BORDER, TEXTURE_HEIGHT, 0);
	glTexCoord2i(1, 1);
	glVertex3f(TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT, 0);
	glTexCoord2i(1, 0);
	glVertex3f(TEXTURE_WIDTH * 2 + BORDER, 0, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void hw1_terminate()
{
	printf("Terminating homework 1 ...\t");
	//Restore the initial size and view port.
	glfwSetWindowSize(TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, -1, 1);
	printf("DONE!\n");
}