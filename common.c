#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

#include <GL/glfw.h>
#include <GL/glext.h>

#include "common.h"

void play_error_sound(void)
{
	PlaySound((LPCTSTR) SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ALIAS_ID | SND_ASYNC);
}

void bitmap_init(GLuint ** data)
{
	*data = (GLuint*) calloc(TEXTURE_WIDTH * TEXTURE_HEIGHT, sizeof(GLuint));

}

void bitmap_fill(GLuint ** data, GLubyte red, GLubyte green, GLubyte blue)
{
	GLint i;

	for (i = 0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT; i++) {
		(*data)[i] = ((red << 24) | (green << 16) | (blue << 8) | (255 << 0));
	}
}

void bitmap_upload(GLuint ** data, GLuint texture_id)
{
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Just in case even though not using mipmaps.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);


	//glTexSubImage2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*) *data);
}

void bitmap_update(GLuint ** data, GLuint texture_id)
{
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*) *data);
}

void bitmap_free(GLuint ** data)
{
	free(*data);
}

void bitmap_set_pixel(GLuint ** data, GLuint x, GLuint y, GLubyte red, GLubyte green, GLubyte blue)
{
	if (x >=  TEXTURE_WIDTH || y >= TEXTURE_HEIGHT)
		return;

	(*data)[(TEXTURE_WIDTH * y) + x] = ((red << 24) | (green << 16) | (blue << 8) | (255 << 0));
}

void bitmap_draw_line(GLuint ** data, GLuint x1, GLuint y1, GLuint x2, GLuint y2, GLubyte red, GLubyte green, GLubyte blue)
{
	GLuint dx, dy;
	GLint sx, sy, err, err2;

	assert((x1 < TEXTURE_WIDTH) && (x2 < TEXTURE_WIDTH) && (y1 < TEXTURE_HEIGHT) && (y2 < TEXTURE_HEIGHT));

	if (x1 < x2) {
		sx = 1;
		dx = x2 - x1;
	} else {
		sx = -1;
		dx = x1 - x2;
	}

	if (y1 < y2) {
		sy = 1;
		dy = y2 - y1;
	} else {
		sy = -1;
		dy = y1 - y2;
	}

	err = dx - dy;

	for (;;)
	{
		bitmap_set_pixel(data, x1, y1, red, green, blue);

		if (x1 == x2 && y1 == y2)
			break;

		err2 = 2 * err;

		if (err2 > -1 * (GLint) dy) {
			err -= dy;
			x1 += sx;
		}

		if (err2 < (GLint) dx) {
			err += dx;
			y1 += sy;
		}
	}
}

void bitmap_draw_circle(GLuint ** data, GLuint x, GLuint y, GLuint radius, GLubyte red, GLubyte green, GLubyte blue)
{
	GLint f = 1 - radius;
	GLint ddF_x = 1;
	GLint ddF_y = -2 * radius;
	GLint cx = 0;
	GLint cy = radius;

	assert((x < TEXTURE_WIDTH) && (y < TEXTURE_HEIGHT));

	bitmap_set_pixel(data, x, y + radius, red, green, blue);
	bitmap_set_pixel(data, x, y - radius, red, green, blue);
	bitmap_set_pixel(data, x + radius, y, red, green, blue);
	bitmap_set_pixel(data, x - radius, y, red, green, blue);

	while(cx < cy)
	{
		// ddF_x == 2 * x + 1;
		// ddF_y == -2 * y;
		// f == x*x + y*y - radius*radius + 2*x - y + 1;
		if(f >= 0) 
		{
			cy--;
			ddF_y += 2;
			f += ddF_y;
		}
		cx++;
		ddF_x += 2;
		f += ddF_x;    
		bitmap_set_pixel(data, x + cx, y + cy, red, green, blue);
		bitmap_set_pixel(data, x - cx, y + cy, red, green, blue);
		bitmap_set_pixel(data, x + cx, y - cy, red, green, blue);
		bitmap_set_pixel(data, x - cx, y - cy, red, green, blue);
		bitmap_set_pixel(data, x + cy, y + cx, red, green, blue);
		bitmap_set_pixel(data, x - cy, y + cx, red, green, blue);
		bitmap_set_pixel(data, x + cy, y - cx, red, green, blue);
		bitmap_set_pixel(data, x - cy, y - cx, red, green, blue);
	}
}