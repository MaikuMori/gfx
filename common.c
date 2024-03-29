#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/glext.h>

#include "common.h"


char* read_file(char *file)
{
    FILE *f;
    long length;
    char *buf;

    f = fopen(file, "rb"); 
    if (!f) 
        return NULL;

    fseek(f, 0, SEEK_END); 
    length = ftell(f);
    buf = (char *) malloc(length+1);
    fseek(f, 0, SEEK_SET); 
    fread(buf, length, 1, f); 
    fclose(f);
    buf[length] = 0;

    return buf;
}

void play_error_sound(void)
{
    PlaySound((LPCTSTR) SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ALIAS_ID | SND_ASYNC);
}

void error( const char* format, ... ) {
    va_list args;
    play_error_sound();
    printf("Error: ");
    va_start(args, format);
    vprintf(format, args);
    va_end( args );
    fprintf( stderr, "\n" );
}

void print_program_log(GLuint obj)
{
    GLint infoLogLength = 0;
    GLsizei charsWritten  = 0;
    GLchar *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        infoLog = (char *) malloc(infoLogLength);
        glGetProgramInfoLog(obj, infoLogLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void print_shader_log(GLuint obj)
{
    GLint infoLogLength = 0;
    GLsizei charsWritten  = 0;
    GLchar *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        infoLog = (char *) malloc(infoLogLength);
        glGetShaderInfoLog(obj, infoLogLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

ShaderProgram * load_shaders(char * vert, char * frag)
{
    ShaderProgram * prog;        
    
    char * vs, * fs;
    const char * vv, * ff;

    vs = read_file(vert);
    fs = read_file(frag);

    if ((vs == NULL) || (fs == NULL)) {
        return NULL;
    }

    prog = (ShaderProgram *) malloc(sizeof(ShaderProgram));

    prog->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    prog->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    vv = vs;
    ff = fs;

    glShaderSource(prog->vertex_shader, 1, &vv,NULL);
    glShaderSource(prog->fragment_shader, 1, &ff,NULL);

    free(vs);
    free(fs);

    glCompileShader(prog->vertex_shader);
    glCompileShader(prog->fragment_shader);

    prog->program = glCreateProgram();

    glAttachShader(prog->program, prog->vertex_shader);
    glAttachShader(prog->program, prog->fragment_shader);

    glLinkProgram(prog->program);

    return prog;
}

void unload_shaders(ShaderProgram * prog)
{
    glDetachShader(prog->program, prog->vertex_shader);
    glDetachShader(prog->program, prog->fragment_shader);

    glDeleteShader(prog->vertex_shader);
    glDeleteShader(prog->fragment_shader);

    glDeleteProgram(prog->fragment_shader);

    free(prog);
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

void hsv2rgb(GLuint hue, GLuint sat, GLuint val, Color * rgb) {

    double h;
    double s;
    double v;

    double r = 0;
    double g = 0;
    double b = 0;

    // Scale Hue to be between 0 and 360. Saturation
    // and value scale to be between 0 and 1.
    h = (double) hue;
    s = (double) sat / 100;
    v = (double) val / 100;

    if (s == 0) {
        // If s is 0, all colors are the same.
        // This is some flavor of gray.
        r = v;
        g = v;
        b = v;
    } else {
        double p;
        double q;
        double t;

        double fractionalSector;
        int sectorNumber;
        double sectorPos;

        // The color wheel consists of 6 sectors.
        // Figure out which sector you're in.
        sectorPos = h / 60;
        sectorNumber = (int)(floor(sectorPos));

        // get the fractional part of the sector.
        // That is, how many degrees into the sector
        // are you?
        fractionalSector = sectorPos - sectorNumber;

        // Calculate values for the three axes
        // of the color. 
        p = v * (1 - s);
        q = v * (1 - (s * fractionalSector));
        t = v * (1 - (s * (1 - fractionalSector)));

        // Assign the fractional colors to r, g, and b
        // based on the sector the angle is in.
        switch (sectorNumber) 
        {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        case 5:
            r = v;
            g = p;
            b = q;
            break;
        }
    }
    
    //Set final colors in rgb struct.
    rgb->red = (int) (r * 255);
    rgb->green = (int) (g * 255);
    rgb->blue = (int) (b * 255);
}