#include "my_math.h"

//Must be power of two.
#define TEXTURE_HEIGHT 512
#define TEXTURE_WIDTH 512

//How many texture are we going to use.
//Not sure if preallocating textures is the best way to go in this case.
//Doing it on per homework bases could be easier and make more sense.
//Or could just write simple resource manager.
// [0] = hw_3_bitmap
// [1] = hw_1_image
// [2] = hw_1_bitmap
// [3] = hw_6_bitmap
// [4] = hw_9_image
// [5] = hw_9_bitmap
// [6] = hw_8_image
// [7] = hw_8_bitmap
// [8] = hw_10_image
// [9] = hw_10_bitmap
// [10] = hw_10_temp1
// [11] = hw_10_temp2 
#define TEXTURE_COUNT 12

//Holds OpenGL texture IDs
GLuint textures[TEXTURE_COUNT];
//Texture buffers.
GLuint * texture_data[TEXTURE_COUNT];

//Types
typedef struct {
    GLubyte red;
    GLubyte green;
    GLubyte blue;
} Color;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
} ShaderProgram;

//Misc functions.

//Read a file into char array.
char* read_file(char *file);

//Feedback functions.
void play_error_sound(void);
void error( const char* format, ... );
void print_program_log(GLuint obj);
void print_shader_log(GLuint obj);

//Shader functions.
ShaderProgram * load_shaders(char * vert, char * frag);
void unload_shaders(ShaderProgram * prog);

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
//Convert from hsv to rgb.
void hsv2rgb(GLuint hue, GLuint sat, GLuint val, Color * rgb);