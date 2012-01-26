#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#include <GL/glfw.h>
#include <GL/glext.h>
#include <stb_image.h>

#include "hw8.h"
#include "common.h"

#define BORDER 5

#define CONSTRAST_STEP 15
#define CONSTRAST_MIN -255
#define CONSTRAST_MAX 255

static GLubyte * image_data;

static GLboolean opening_file;
static GLboolean init_done;

static GLint constrast = 0;

static void apply_constrast(void) {
    int i, j, x, y;
    unsigned char c[3];

    for (i = 0, x = 0, y = 0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT * 4; i += 4, x++) {
        if (x == TEXTURE_WIDTH) {
            y++;
            x = 0;
        }

        for (j = 0; j < 3; ++j) {
            if (image_data[i+j] < constrast) {
                c[j] = 0;
            } else if (image_data[i+j] > 255 - constrast) {
                c[j] = 255;
            } else {
                c[j] = (unsigned char) ((float)(image_data[i+j] - constrast) / (255.0f - constrast * 2) * 255.0f);
                //c[j] = (unsigned char) ((image_data[i+j] - 128) * constrast + 128);
            }
        }

        bitmap_set_pixel(&texture_data[7], x, y, c[0], c[1], c[2]);

    }

    bitmap_update(&texture_data[7], textures[7]);
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

    glBindTexture(GL_TEXTURE_2D, textures[6]);

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
            apply_constrast();
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
        if (constrast > CONSTRAST_MIN) {
            constrast -= CONSTRAST_STEP;
            apply_constrast();
        }
        break;
    case 'W':
        if (constrast < CONSTRAST_MAX) {
            constrast += CONSTRAST_STEP;
            apply_constrast();
        }
        break;
    case 'R':
        constrast = 0;
        apply_constrast();
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
    printf(" Use keys [Q, W, R] to manipulate the image.\n");
    printf(" \t'Q' - Decrease contrast\n");
    printf(" \t'W' - Increase contrast\n");
    printf(" \t'R' - Reset contrast\n");
}

void hw8_init(void)
{
    printf("\nInitializing homework 8 ...\n");

    //Set defaults.
    image_data = NULL;
    init_done = GL_FALSE;

    glfwSetWindowTitle("GFX Homework: 8.a");


    //Load the test image.
    if (!load_image("Images/test2.png")) {
        return;
    }

    bitmap_fill(&texture_data[7], 255, 255, 255);
    bitmap_upload(&texture_data[7], textures[7]);

    apply_constrast();
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

void hw8_draw(void)
{
    if (!init_done) {
        return;
    }

    //Draw quads and texture them using our bitmaps.

    //The picture.
    glBindTexture(GL_TEXTURE_2D, textures[6]);
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
    glBindTexture(GL_TEXTURE_2D, textures[7]);
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

void hw8_terminate(void)
{
    //Remove key handler.
    glfwSetKeyCallback(NULL);

    //Restore OpenGL settings.
    glDisable(GL_TEXTURE_2D);

    //Free memory.
    if (image_data != NULL)
        stbi_image_free(image_data);
}