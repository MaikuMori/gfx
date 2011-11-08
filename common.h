#include <GL/glfw.h>
#include <GL/glext.h>

//Must be power of two.
#define TEXTURE_HEIGHT 512
#define TEXTURE_WIDTH 512

//How many texture are we going to use.
//Not sure if preallocating textures is the best way to go in this case.
//Doing it on per homework bases could be easier and make more sense.
// [0] = hw_3_bitmap
// [1] = hw_1_image
// [2] = hw_1_bitmap
#define TEXTURE_COUNT 3

//Holds OpenGL texture IDs
GLuint textures[TEXTURE_COUNT];
//Texture buffers.
GLuint * texture_data[TEXTURE_COUNT];

//Types
typedef struct {
	GLint x;
	GLint y;
} Vector2i;

typedef struct {
	GLfloat x;
	GLfloat y;
} Vector2f;

typedef struct {
	GLubyte red;
	GLubyte green;
	GLubyte blue;
} Color;

//Functions to directly work with textures (bitmaps).

//Initialize bitmap.
void bitmap_init(GLuint ** data);
//Fill whole bitmap with a specific color, can be used to just erase all the content.
void bitmap_fill(GLuint ** data, GLubyte red, GLubyte green, GLubyte blue);
//Upload the bitmap to OpenGL (VRAM) and set the required parameters.
void bitmap_upload(GLuint ** data, GLuint texture_id);
//Update the bitmap, a bit less overhead than re-uploading.
void bitmap_update(GLuint ** data, GLuint texture_id);
//Free the bitmap.
void bitmap_free(GLuint ** data);

//Set a specific pixel (x, y) in the bitmap to a specific color.
void bitmap_set_pixel(GLuint ** data, GLuint x, GLuint y, GLubyte red, GLubyte green, GLubyte blue);

//Draw a line on bitmap from (x1, y1) to (x2, y2).
void bitmap_draw_line(GLuint ** data, GLuint x1, GLuint y1, GLuint x2, GLuint y2, GLubyte red, GLubyte green, GLubyte blue);
//Draw a circle on bitmap with center (x, y).
void bitmap_draw_circle(GLuint ** data, GLuint x, GLuint y, GLuint radius, GLubyte red, GLubyte green, GLubyte blue);