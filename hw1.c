#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <GL/glfw.h>
#include <GL/glext.h>
#include <stb_image.h>

#include "hw1.h"
#include "common.h"

#define BORDER 5
#define HISTOGRAM_TOP_GAP 50
#define HISTOGRAM_BUCKETS 256

#define HISTOGRAM_NONE            0x00000000
#define HISTOGRAM_RED            0x00000001
#define HISTOGRAM_GREEN            0x00000002
#define HISTOGRAM_BLUE            0x00000004
#define HISTOGRAM_LUMINANACE    0x00000008
#define HISTOGRAM_RGB (HISTOGRAM_RED | HISTOGRAM_GREEN | HISTOGRAM_BLUE)

static GLubyte * image_data;
static GLuint  * histogram_data;

static GLenum current_histogram;

static GLboolean opening_file;

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

    glBindTexture(GL_TEXTURE_2D, textures[1]);

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

static void draw_histogram(GLenum type)
{
    GLuint i, max_count;    

    //Clear the old data.
    memset(histogram_data, 0, HISTOGRAM_BUCKETS * sizeof(int));
    //Clear the bitmap.
    bitmap_fill(&texture_data[2], 255, 255, 255);

    //Count the pixel values.
    for (i=0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT * 4; i+=4) {
        if (type & HISTOGRAM_RED)
            histogram_data[image_data[i+0]]++;
        if (type & HISTOGRAM_BLUE)
            histogram_data[image_data[i+1]]++;
        if (type & HISTOGRAM_GREEN)
            histogram_data[image_data[i+2]]++;
        if (type & HISTOGRAM_LUMINANACE)
            //Luminance (perceived). Alternative: Luminance (standard, objective): (0.2126*R) + (0.7152*G) + (0.0722*B)
            histogram_data[((image_data[i+0] * 299) + (image_data[i+1]  * 587) + (image_data[i+2] * 114)) / 1000]++;
    }

    //Find the highest value to normalize the graph.
    max_count = 0;
    for (i=0; i < HISTOGRAM_BUCKETS; i++) {
        if (histogram_data[i] > max_count)
            max_count = histogram_data[i];
    }
    //max_count = TEXTURE_WIDTH * TEXTURE_HEIGHT * 4;
    //Draw the histogram.
    for (i=0; i < TEXTURE_WIDTH; i+=2) {
        //Going to break if TEXTURE_WIDTH != 512. But since the sizes are constant for now, it shouldn't be a problem.
        bitmap_draw_line(&texture_data[2], i, TEXTURE_HEIGHT-1, i,
            TEXTURE_HEIGHT - 1 - (GLuint) (((float) histogram_data[i/2] / (float) max_count) * (TEXTURE_HEIGHT - HISTOGRAM_TOP_GAP)),
            (type & HISTOGRAM_RED   && type ^ HISTOGRAM_RGB) ? 255 : 0,
            (type & HISTOGRAM_GREEN && type ^ HISTOGRAM_RGB) ? 255 : 0,
            (type & HISTOGRAM_BLUE  && type ^ HISTOGRAM_RGB) ? 255 : 0);
        bitmap_draw_line(&texture_data[2], i+1, TEXTURE_HEIGHT-1, i+1,
            TEXTURE_HEIGHT - 1 - (GLuint) (((float) histogram_data[i/2] / (float) max_count) * (TEXTURE_HEIGHT - HISTOGRAM_TOP_GAP)),
            (type & HISTOGRAM_RED   && type ^ HISTOGRAM_RGB) ? 255 : 0,
            (type & HISTOGRAM_GREEN && type ^ HISTOGRAM_RGB) ? 255 : 0,
            (type & HISTOGRAM_BLUE  && type ^ HISTOGRAM_RGB) ? 255 : 0);
    }

    //Update the texture.
    bitmap_update(&texture_data[2], textures[2]);
}

static void open_file(void)
{
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];

    size_t file_len;
    size_t converted_len = 0;
    char converted_filename[MAX_PATH];

    opening_file = GL_TRUE;

    szFile[0] = '\0';
    szFile[1] = '\0';

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
            draw_histogram(current_histogram);
        }
    }

    opening_file = GL_FALSE;
}

static void GLFWCALL key_handler(int key, int action)
{
    //No need to do this for both press and release.
    if (action == GLFW_RELEASE)
        return;

    //Switch histogram type based on keys (Q, W, E, R, T). Show open image dialog (O).
    switch (key) {
    case 'Q':
        if (current_histogram != HISTOGRAM_RGB)    {
            current_histogram = HISTOGRAM_RGB;
            draw_histogram(HISTOGRAM_RGB);
        }
        break;
    case 'W':
        if (current_histogram != HISTOGRAM_RED)    {
            current_histogram = HISTOGRAM_RED;
            draw_histogram(HISTOGRAM_RED);
        }
        break;
    case 'E':
        if (current_histogram != HISTOGRAM_GREEN)    {
            current_histogram = HISTOGRAM_GREEN;
            draw_histogram(HISTOGRAM_GREEN);
        }
        break;
    case 'R':
        if (current_histogram != HISTOGRAM_BLUE)    {
            current_histogram = HISTOGRAM_BLUE;
            draw_histogram(HISTOGRAM_BLUE);
        }
        break;
    case 'T':
        if (current_histogram != HISTOGRAM_LUMINANACE)    {
            current_histogram = HISTOGRAM_LUMINANACE;
            draw_histogram(HISTOGRAM_LUMINANACE);
        }
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
    printf(" Use keys [Q-T] to toggle between different histograms.\n");
    printf(" \t'Q' - RGB histogram\n");
    printf(" \t'W' - Red channel histogram\n");
    printf(" \t'E' - Green channel histogram\n");
    printf(" \t'R' - Blue channel histogram\n");
    printf(" \t'T' - Luminance (perceived) histogram\n");
}

void hw1_init(void)
{
    printf("\nInitializing homework 1 ...\n");

    //Set defaults.
    image_data = NULL;
    current_histogram = HISTOGRAM_NONE;
    histogram_data = (GLuint*) calloc(HISTOGRAM_BUCKETS, sizeof(GLuint));

    glfwSetWindowTitle("GFX Homework: 1.a");

    //Color the histogram background white and upload it.
    bitmap_fill(&texture_data[2], 255, 255, 255);
    bitmap_upload(&texture_data[2], textures[2]);

    //Load the test image.
    if (load_image("Images/test.png")) {
        current_histogram = HISTOGRAM_RGB;
        draw_histogram(HISTOGRAM_RGB);
    }

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

    print_help();
}

void hw1_draw(void)
{
    //Draw quads and texture them using our bitmaps.

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
}

void hw1_terminate(void)
{
    //Remove key handler.
    glfwSetKeyCallback(NULL);

    //Restore OpenGL settings.
    glDisable(GL_TEXTURE_2D);

    //Free memory.
    if (image_data != NULL)
        stbi_image_free(image_data);

    if (histogram_data != NULL)
        free(histogram_data);
}