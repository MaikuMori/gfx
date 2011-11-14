#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include <GL/glew.h>
//Should probably not use this one, but I don't ant to calculate the matrices
//manually. And this way I can support OpenGL 1.5.
#include <GL/glu.h>
#include <GL/glfw.h>
#include <GL/glext.h>

#include "hw2.h"
#include "common.h"

//Mouse buttons.
#define LEFT_BUTTON     0
#define RIGHT_BUTTON    1

//64x64x64 seems to be the limit on my PC since we're not using any proper optimization.
#define VOXEL_COUNT_X 8
#define VOXEL_COUNT_Y 8
#define VOXEL_COUNT_Z 8
#define VOXEL_COUNT VOXEL_COUNT_X * VOXEL_COUNT_Y * VOXEL_COUNT_Z

GLboolean glew_initialized = 0;
GLboolean mouse_button[2];
Vector2i mouse_position;
Vector2f camera_position;

GLuint voxelVBO;
#pragma region VoxelData
//Cube data.
GLfloat vertices[] = {1,1,1,  -1,1,1,  -1,-1,1,  1,-1,1,      
    1,1,1,  1,-1,1,  1,-1,-1,  1,1,-1,        
    1,1,1,  1,1,-1,  -1,1,-1,  -1,1,1,        
    -1,1,1,  -1,1,-1,  -1,-1,-1,  -1,-1,1,    
    -1,-1,-1,  1,-1,-1,  1,-1,1,  -1,-1,1,    
    1,-1,-1,  -1,-1,-1,  -1,1,-1,  1,1,-1};
//Wrote it with lighting before, but since the color picker map looked so nice
//I removed the lights, so it theory the next two arrays are worthless, but I'll
//Just keep them around just in case I need it later.
GLfloat normals[] = {0,0,1,  0,0,1,  0,0,1,  0,0,1,            
    1,0,0,  1,0,0,  1,0,0, 1,0,0,              
    0,1,0,  0,1,0,  0,1,0, 0,1,0,              
    -1,0,0,  -1,0,0, -1,0,0,  -1,0,0,          
    0,-1,0,  0,-1,0,  0,-1,0,  0,-1,0,         
    0,0,-1,  0,0,-1,  0,0,-1,  0,0,-1};        
GLfloat colors[] = {1,1,1,  1,1,0,  1,0,0,  1,0,1,            
    1,1,1,  1,0,1,  0,0,1,  0,1,1,             
    1,1,1,  0,1,1,  0,1,0,  1,1,0,             
    1,1,0,  0,1,0,  0,0,0,  1,0,0,             
    0,0,0,  0,0,1,  1,0,1,  1,0,0,             
    0,0,1,  0,0,0,  0,1,0,  0,1,1};
#pragma endregion VoxelData

unsigned char voxel_map[VOXEL_COUNT_X][VOXEL_COUNT_Y][VOXEL_COUNT_Z];

//Spacing between the voxels.
GLfloat voxel_spacing = 1.0;

//Shaders.
ShaderProgram * simple_color;
//ShaderProgram * phong;
GLint obj_color_location;

static void move_camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat targetX, GLfloat targetY, GLfloat targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
}

static void draw_object(void)
{
    GLuint x, y, z;
    GLfloat obj_color[4];

    glPushMatrix();

    //We're actually rotating the object :).
    glRotatef(camera_position.x, 1, 0, 0);
    glRotatef(camera_position.y, 0, 1, 0);

    glTranslatef((VOXEL_COUNT_X / 2) * - (2 + voxel_spacing),
        (VOXEL_COUNT_Y / 2) * - (2 + voxel_spacing),
        (VOXEL_COUNT_Z / 2) * - (2 + voxel_spacing));

    glBindBuffer(GL_ARRAY_BUFFER, voxelVBO);

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    //Render the object on the fly.
    for (z=0; z < VOXEL_COUNT_Z; z++) {
        for (y=0; y < VOXEL_COUNT_Y; y++) {
            for (x=0; x < VOXEL_COUNT_X; x++) {
                glTranslatef((2 + voxel_spacing), 0, 0);

                if(!voxel_map[x][y][z]) {
                    continue;
                }
                
                //Set the voxel color.
                obj_color[0] = (GLfloat) x / (VOXEL_COUNT_X - 1);
                obj_color[1] = (GLfloat) y / (VOXEL_COUNT_Y - 1);
                obj_color[2] = (GLfloat) z / (VOXEL_COUNT_Z - 1);
                obj_color[3] = 1.0f;
                glUniform4fv(obj_color_location, 1, obj_color);
                //Draw the voxel.
                glDrawArrays(GL_QUADS, 0, 24);
            }
            glTranslatef(VOXEL_COUNT_X * -(2 + voxel_spacing), (2 + voxel_spacing), 0);
        }
        glTranslatef(0, VOXEL_COUNT_Y * -(2 + voxel_spacing), (2 + voxel_spacing));
    }
    //Don't need to but just to play along.
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPopMatrix();
}

static void GLFWCALL mouse_click_handler(int button, int action)
{
    GLfloat pixel[3];
    GLint viewport[4];
    int x, y;
    unsigned char ix, iy, iz;

    mouse_button[button] = (GLboolean) action;

    if(mouse_button[LEFT_BUTTON]) {
        //Get the pixel at mouse position.
        glfwGetMousePos(&x, &y);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_FLOAT, pixel);
        
        //Find the indexes. There must be a better way.

        ix = (unsigned char) (pixel[0] * (VOXEL_COUNT_X));
        iy = (unsigned char) (pixel[1] * (VOXEL_COUNT_Y));
        iz = (unsigned char) (pixel[2] * (VOXEL_COUNT_Z));
        
        //Nasty trick to prevent deletion of the black voxel in case user clicks
        //on the background.
        if ((ix == 0) && (iy == 0) && (iz == 0)) {
            if ((pixel[0] != 0.0f) && (pixel[1] != 0.0f) && (pixel[2] != 0.0f) ) {
                return;
            }
        }

        if(ix == VOXEL_COUNT_X)
            ix = VOXEL_COUNT_X - 1;
        if(iy == VOXEL_COUNT_Y)
            iy = VOXEL_COUNT_Y - 1;
        if(iz == VOXEL_COUNT_Z)
            iz = VOXEL_COUNT_Z - 1;

        //"Delete" the voxel.
        voxel_map[ix][iy][iz] = 0;
    }
}

static void GLFWCALL mouse_pos_handler(int x, int y)
{
    if(mouse_button[RIGHT_BUTTON]) {
        camera_position.y += (x - mouse_position.x);
        camera_position.x += (y - mouse_position.y);
        mouse_position.x = x;
        mouse_position.y = y;
    }
}

static void GLFWCALL key_handler(int key, int action)
{
    //No need to do this for both press and release.
    if (action == GLFW_PRESS)
        return;

    switch (key) {
    case 'Q':
        //Decrease spacing.
        if (voxel_spacing > 0)    {
            voxel_spacing -= 0.1f;
        }
        break;
    case 'W':
        //Increase spacing.
        if (voxel_spacing < 3)    {
            voxel_spacing += 0.1f;
        }
        break;
    case 'R':
        //Reset the voxel data.
        memset(voxel_map, 1, VOXEL_COUNT);
        break;
    }
}

void hw2_init(void)
{
    printf("Initializing homework 2 ...\t");
    glfwSetWindowTitle("GFX Homework: 2.1.a");

    //Initialize defaults.
    mouse_button[LEFT_BUTTON] = 0;
    mouse_button[RIGHT_BUTTON] = 0;
    camera_position.x = 25;
    camera_position.y = 45;
    memset(voxel_map, 1, VOXEL_COUNT);

    //Add callbacks.
    glfwSetMouseButtonCallback(mouse_click_handler);
    glfwSetMousePosCallback(mouse_pos_handler);
    glfwSetKeyCallback(key_handler);

    //Initialize glew if needed. (Helps loading OpenGL extensions).
    if (!glew_initialized) {
        if (GLEW_OK == glewInit()) {
            glew_initialized = GL_TRUE;
        } else {
            play_error_sound();
            printf("ERROR: Failed to initialize glew.");
        }
    }

    //Check for OpenGL 2.1+
    if(!GLEW_VERSION_2_1) {
        play_error_sound();
        printf("Error: OpenGL 2.1+ required.");
    }

    //Setup OpenGL.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(TEXTURE_WIDTH) / TEXTURE_HEIGHT, 1.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.01f, 0.01f, 0.01f, 0);
    glClearStencil(0);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);

    //Move camera.
    move_camera(0, 0, (GLfloat)(VOXEL_COUNT_Z * voxel_spacing + (VOXEL_COUNT_X * 4.5)), 0, 0, 0);

    //Generate the cube VBO.
    glGenBuffers(1, &voxelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, voxelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);                             
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);                
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);

    glNormalPointer(GL_FLOAT, 0, (void*)sizeof(vertices));
    glColorPointer(3, GL_FLOAT, 0, (void*)(sizeof(vertices)+sizeof(normals)));
    glVertexPointer(3, GL_FLOAT, 0, 0);

    //Load shaders.
    //phong = load_shaders("Shaders/phong.vert", "Shaders/phong.frag");
    simple_color = load_shaders("Shaders/color.vert", "Shaders/color.frag");

    glUseProgram(simple_color->program);
    //Get the obj_color variable location in color_picker shader.
    obj_color_location = glGetUniformLocation(simple_color->program, "obj_color");
    
    printf("DONE!\n");

    printf("Using %d voxels!\n", VOXEL_COUNT);
}

void hw2_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    glUseProgram(simple_color->program);
    //Render the object.
    draw_object();
}

void hw2_terminate(void)
{
    printf("Terminating homework 2 ...\t");

    //Remove callbacks.
    glfwSetMouseButtonCallback(NULL);
    glfwSetMousePosCallback(NULL);
    glfwSetKeyCallback(NULL);

    //Restore OpenGL.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    //unload_shaders(phong);
    unload_shaders(simple_color);

    glUseProgram(0);

    //Reset matrices.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Delete the VBO.
    glDeleteBuffers(1, &voxelVBO);

    printf("DONE!\n");
}