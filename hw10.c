#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#include <GL/glfw.h>
#include <GL/glext.h>
#include <stb_image.h>

#include "hw9.h"
#include "common.h"

#define BORDER 5

static GLubyte * image_data;

static GLboolean opening_file;
static GLboolean init_done;

//9x9 Laplacian of Gaussian (Gaussian smoothing + Laplacian)
static GLdouble LoG_kernel[9 * 9] = {
    0/40.0f, 1/40.0f, 1/40.0f, 2/40.0f, 2/40.0f, 2/40.0f, 1/40.0f, 1/40.0f, 0/40.0f,
    1/40.0f, 2/40.0f, 4/40.0f, 5/40.0f, 5/40.0f, 5/40.0f, 4/40.0f, 2/40.0f, 1/40.0f,
    1/40.0f, 4/40.0f, 5/40.0f, 3/40.0f, 0/40.0f, 3/40.0f, 5/40.0f, 4/40.0f, 1/40.0f,
    2/40.0f, 5/40.0f, 3/40.0f, -12/40.0f, -24/40.0f, -12/40.0f, 3/40.0f, 5/40.0f, 2/40.0f,
    2/40.0f, 5/40.0f, 0/40.0f, -24/40.0f, -40/40.0f, -24/40.0f, 0/40.0f, 5/40.0f, 2/40.0f,
    2/40.0f, 5/40.0f, 3/40.0f, -12/40.0f, -24/40.0f, -12/40.0f, 3/40.0f, 5/40.0f, 2/40.0f,
    1/40.0f, 4/40.0f, 5/40.0f, 3/40.0f, 0/40.0f, 3/40.0f, 5/40.0f, 4/40.0f, 1/40.0f,
    1/40.0f, 2/40.0f, 4/40.0f, 5/40.0f, 5/40.0f, 5/40.0f, 4/40.0f, 2/40.0f, 1/40.0f,
    0/40.0f, 1/40.0f, 1/40.0f, 2/40.0f, 2/40.0f, 2/40.0f, 1/40.0f, 1/40.0f, 0/40.0f,
};

//3x3 Laplacian kernel.
static GLdouble laplacian_kernel[9] = {
    0, -1, 0,
    -1, 4, -1,
    0, -1, 0
};

//3x3 Sobel x kernel.
static GLdouble sobel_x_kernel[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
};

//3x3 Sobel y kernel.
static GLdouble sobel_y_kernel[9] = {
    -1, -2, -1,
    0, 0, 0,
    1, 2, 1
};

//Very simple convolution implementation (also extreamly slow).
static void bitmap_apply_kernel(GLuint ** data_out, GLubyte * data_in, GLdouble * kernel, GLbyte rows, GLbyte cols)
{
    GLbyte c_x = cols / 2;
    GLbyte c_y = rows / 2;
    GLint i, j, m, n, m_i, n_i;
    GLint x, y;
    GLdouble sum_r, sum_g, sum_b;

    for (i = 0; i < TEXTURE_HEIGHT; ++i) {
        for (j = 0; j < TEXTURE_WIDTH; ++j) {
            sum_r = sum_g = sum_b = 0;

            for (m = 0; m < rows; ++m) {
                m_i = rows - m - 1;

                for (n = 0; n < cols; ++n) {
                    n_i = cols - n - 1;

                    //Data indexes.
                    x = j + n + c_x;
                    y = i + m + c_y;

                    //Check bounds.
                    if(y >= 0 && y < TEXTURE_HEIGHT && x >= 0 && x < TEXTURE_WIDTH) {
                        x *= 4;
                        y *= 4;
                        sum_r += data_in[TEXTURE_WIDTH * y + x + 0] * kernel[cols * m_i + n_i];
                        sum_g += data_in[TEXTURE_WIDTH * y + x + 1] * kernel[cols * m_i + n_i];
                        sum_b += data_in[TEXTURE_WIDTH * y + x + 2] * kernel[cols * m_i + n_i];
                    }
                }
            }

            if (sum_r < 0) sum_r = 0;
            if (sum_r > 255) sum_r = 255;
            if (sum_g < 0) sum_g = 0;
            if (sum_g > 255) sum_g = 255;
            if (sum_b < 0) sum_b = 0;
            if (sum_b > 255) sum_b = 255;

            bitmap_set_pixel(data_out, j, i,
                (GLbyte)((double) fabs(sum_r) + 0.5f),
                (GLbyte)((double) fabs(sum_g) + 0.5f),
                (GLbyte)((double) fabs(sum_b) + 0.5f)
                );
        }
    }

}

static void bitmap_combine(GLuint ** bitmap_out, GLuint ** bitmap_in1, GLuint ** bitmap_in2)
{
    GLint i;
    GLint r, r1, r2, g, g1, g2, b, b1, b2;

    for (i = 0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT; i++) {
        // :(
        r1 = ((*bitmap_in1)[i] >> 24) & 0xFF;
        r2 = ((*bitmap_in2)[i] >> 24) & 0xFF;
        g1 = ((*bitmap_in1)[i] >> 16) & 0xFF;
        g2 = ((*bitmap_in2)[i] >> 16) & 0xFF;
        b1 = ((*bitmap_in1)[i] >> 8) & 0xFF;
        b2 = ((*bitmap_in2)[i] >> 8) & 0xFF;

        r = (int) sqrt((float)r1 * r1 + r2 * r2);
        g = (int) sqrt((float)g1 * g1 + g2 * g2);
        b = (int) sqrt((float)b1 * b1 + b2 * b2);

        // No more time left :(.
        if (r < 0) r = 0;
        if (r > 255) r = 255;
        if (g < 0) g = 0;
        if (g > 255) g = 255;
        if (b < 0) b = 0;
        if (b > 255) b = 255;

        (*bitmap_out)[i] = ((r << 24) | (g << 16) | (b << 8) | (255 << 0));
    }
}

static GLboolean load_image(char * filename)
{
    GLint x,y,n;

    if (image_data != NULL)
    {
        stbi_image_free(image_data);
    }

    image_data = stbi_load(filename, &x, &y, &n, 4);

    if (image_data == NULL) {
        play_error_sound();
        printf("Error: Failed to open image: %s.\n", filename);
        return GL_FALSE;
    }

    if (x != TEXTURE_WIDTH || y !=TEXTURE_HEIGHT)
    {
        play_error_sound();
        printf("Error: Image should be %dpx x %dpx. Scaling isn't currently supported.\n", TEXTURE_WIDTH, TEXTURE_HEIGHT);
        return GL_FALSE;
    }

    glBindTexture(GL_TEXTURE_2D, textures[8]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //Just in case even though not using mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, n, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) image_data);

    return GL_TRUE;
}

static void open_file(void)
{
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];
    TCHAR cur_dir[FILENAME_MAX];

    size_t file_len;
    size_t converted_len = 0;
    char converted_filename[MAX_PATH];

    opening_file = GL_TRUE;

    szFile[0] = '\0';
    szFile[1] = '\0';

    //Get current working directory.
    GetCurrentDirectory(sizeof(TCHAR) * FILENAME_MAX, cur_dir);

    //Initialize OPENFILENAME
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = TEXT("Images (*.jpg;*.png;*.bmp;*.tga;*.psd)\0*.jpg;*.png;*.bmp;*.tga;*.psd\0");
    ofn.lpstrInitialDir = TEXT("Images");
    ofn.lpstrTitle = TEXT("Open 512x512 image ...");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn))
    {
        file_len = wcslen(ofn.lpstrFile) + 1;

        wcstombs_s(&converted_len, converted_filename, file_len, ofn.lpstrFile, _TRUNCATE);

        if (converted_len != file_len) {
            play_error_sound();
            printf("Error: Can't open unicode paths.");
            opening_file = GL_FALSE;
            return;
        }

        if(load_image(converted_filename)) {
            bitmap_apply_kernel(&texture_data[9], image_data, LoG_kernel, 9, 9);
            bitmap_update(&texture_data[9], textures[9]);
        }
    }
    //Set the old working directory.
    SetCurrentDirectory(cur_dir);
    opening_file = GL_FALSE;
}

static void GLFWCALL key_handler(int key, int action)
{
    if (!init_done) {
        return;
    }

    //No need to do this for both press and release.
    if (action == GLFW_RELEASE)
        return;

    switch (key) {
    case 'Q':
        bitmap_apply_kernel(&texture_data[9], image_data, LoG_kernel, 9, 9);
        bitmap_update(&texture_data[9], textures[9]);
        break;
    case 'W':
        bitmap_apply_kernel(&texture_data[9], image_data, laplacian_kernel, 3, 3);
        bitmap_update(&texture_data[9], textures[9]);
        break;
    case 'E':
        bitmap_apply_kernel(&texture_data[9], image_data, sobel_x_kernel, 3, 3);
        bitmap_update(&texture_data[9], textures[9]);
        break;
    case 'R':
        bitmap_apply_kernel(&texture_data[9], image_data, sobel_y_kernel, 3, 3);
        bitmap_update(&texture_data[9], textures[9]);
        break;
    case 'T':
        //Apply sobel x.
        bitmap_apply_kernel(&texture_data[10], image_data, sobel_x_kernel, 3, 3);
        //Apply sobel y.
        bitmap_apply_kernel(&texture_data[11], image_data, sobel_y_kernel, 3, 3);
        //Combine.
        bitmap_combine(&texture_data[9], &texture_data[10], &texture_data[11]);
        bitmap_update(&texture_data[9], textures[9]);
        break;
    case 'L':
        if(!opening_file)
            open_file();
        break;
    }
}

static void print_help(void)
{
    printf("\nHelp:\n");
    printf(" Press 'L' to load custom image. It must be %dpx x %dpx in size.\n", TEXTURE_WIDTH, TEXTURE_HEIGHT);
    printf(" There are some example images included.\n\n");
    printf(" Use keys [Q, W, E, R, T] to manipulate the image.\n");
    printf(" \t'Q' - Toggle Laplacian of Gaussian kernel\n");
    printf(" \t'W' - Toggle Laplacian kernel\n");
    printf(" \t'E' - Toggle Sobel x kernel\n");
    printf(" \t'R' - Toggle Sobel y kernel\n");
    printf(" \t'T' - Toggle Sobel x kernel + Sobel y kernel\n");
}

void hw10_init(void)
{
    printf("\nInitializing homework 10 ...\n");

    //Set defaults.
    image_data = NULL;
    init_done = GL_FALSE;

    glfwSetWindowTitle("GFX Homework: 10.c");

    //Load the test image.
    if (!load_image("Images/test2.png")) {
        return;
    }
    bitmap_apply_kernel(&texture_data[9], image_data, LoG_kernel, 9, 9);
    bitmap_upload(&texture_data[9], textures[9]);

    //Add key callback.
    glfwSetKeyCallback(key_handler);

    //Change the window size and OpenGL viewport.
    glfwSetWindowSize(TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT);
    glViewport(0, 0, TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT);
    //Reset projection matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //Set up orthographic projection.
    glOrtho(0, TEXTURE_WIDTH * 2 + BORDER, TEXTURE_HEIGHT, 0, -1, 1);
    //Reset model view matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //Enable textures globally since we're just drawing two textured quads.
    glEnable(GL_TEXTURE_2D);

    printf("DONE!\n");
    init_done = GL_TRUE;

    print_help();
}

void hw10_draw(void)
{
    if (!init_done) {
        return;
    }

    //Draw quads and texture them using our bitmaps.

    //The picture.
    glBindTexture(GL_TEXTURE_2D, textures[8]);
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
    glBindTexture(GL_TEXTURE_2D, textures[9]);
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
}

void hw10_terminate(void)
{
    //Remove key handler.
    glfwSetKeyCallback(NULL);

    //Restore OpenGL settings.
    glDisable(GL_TEXTURE_2D);

    //Free memory.
    if (image_data != NULL)
        stbi_image_free(image_data);
}