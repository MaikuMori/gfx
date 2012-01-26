#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/glext.h>

#include "hw4.h"
#include "common.h"

#define OBJ_FACE_COUNT 12

static boolean init_done;

static ShaderProgram * transformation_shader;

#pragma region DodecahedronData
static const GLfloat vertices[] = {
    -0.57735f,    -0.57735f,    0.57735f,
     0.934172f,    0.356822f,   0.0f,
     0.934172f,   -0.356822f,   0.0f,
    -0.934172f,    0.356822f,   0.0f,
    -0.934172f,   -0.356822f,   0.0f,
     0.0f,         0.934172f,   0.356822f,
     0.0f,         0.934172f,  -0.356822f,
     0.356822f,    0.0f,       -0.934172f,
    -0.356822f,    0.0f,       -0.934172f,
     0.0f,        -0.934172f,  -0.356822f,
     0.0f,        -0.934172f,   0.356822f,
     0.356822f,    0.0f,        0.934172f, 
    -0.356822f,    0.0f,        0.934172f,
     0.57735f,     0.57735f,   -0.57735f,
     0.57735f,     0.57735f,    0.57735f,
    -0.57735f,     0.57735f,   -0.57735f,
    -0.57735f,     0.57735f,    0.57735f,
     0.57735f,    -0.57735f,   -0.57735f,
     0.57735f,    -0.57735f,    0.57735f,
    -0.57735f,    -0.57735f,   -0.57735f
};
static const GLshort indices[] =
{
    1, 2, 18, 11, 14, 
    1, 13, 7, 17, 2,
    3, 4, 19, 8, 15,
    3, 16, 12, 0, 4,
    3, 15, 6, 5, 16,
    1, 14, 5, 6, 13,
    2, 17, 9, 10, 18, 
    4, 0, 10, 9, 19,
    7, 8, 19, 9, 17,
    6, 15, 8, 7, 13,
    5, 14, 11, 12, 16,
    10, 0, 12, 11, 18
};
#pragma endregion DodecahedronData

static GLuint obj_vao;
static GLuint obj_vbo;
static GLuint index_vbo;

static const float ft_near = 1.0f;
static const float ft_far = 1000.0f;
static const float fov = 45.0f;

static GLuint model_matrix_uniform;
static GLuint view_matrix_uniform;
static GLuint pojection_matrix_uniform;
static GLuint color_uniform;

static Matrix4f model_matrix;
static Matrix4f view_matrix;
static Matrix4f projection_matrix;

static void print_help(void)
{
    printf("\nHelp:\n");
    printf(" Nothing special here just rotating dodecahedron.\n");
    printf(" All projections and transformations are done 'manually'.\n");
    printf(" Matrices are generated in the program and then they're multiplied in\n");
    printf(" vertex shader 'Shaders\\transform.vert' and applied to each vertex.\n");
}

void hw4_init(void)
{
    float eye[3]    = {0.0f, 0.0f, -5.0f};
    float center[3] = {0.0f, 0.0f, 0.0f};
    float up[3]     = {0.0f, 0.0f, 0.0f};

    printf("\nInitializing homework 4 ...\n");

    glfwSetWindowTitle("GFX Homework: 4");

    //Check for OpenGL 2.1+
    if(!GLEW_VERSION_2_1) {
        error("OpenGL 2.1+ required to view this homework.");
        return;
    }

    //Setup OpenGL.

    //Change the window size and OpenGL viewport.
    glfwSetWindowSize(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

    //Going to need depth test and we're going to cull faces to improve performance.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 0);
    glClearStencil(0);
    glClearDepth(1.0f);

    //Make the Vertex Buffer Object for objects vertexes and copy vertices data.
    glGenBuffers(1, &obj_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //Make the Vertex Buffer Object for objects indices and copy indices data.
    glGenBuffers(1, &index_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //Set vertexes as input 0.
    glBindBuffer(GL_ARRAY_BUFFER, obj_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
    glBindVertexArray(0);

    //Load Shaders.
    transformation_shader = load_shaders("Shaders/transform.vert", "Shaders/transform.frag");

    if(transformation_shader == NULL) {
        error("Failed to load shaders.");
        return;
    }

    //Get uniform locations.
    model_matrix_uniform = glGetUniformLocation(transformation_shader->program, "model_matrix");
    view_matrix_uniform = glGetUniformLocation(transformation_shader->program, "view_matrix");
    pojection_matrix_uniform = glGetUniformLocation(transformation_shader->program, "projection_matrix");
    //And the color for more fun.
    color_uniform = glGetUniformLocation(transformation_shader->program, "color");

    //Use the shader.
    glUseProgram(transformation_shader->program);

    //Initialize the matrices.
    memcpy(model_matrix, IDENTITY_4F, sizeof(float) * 16);
    memcpy(view_matrix, IDENTITY_4F, sizeof(float) * 16);
    perspective(&projection_matrix, fov, TEXTURE_HEIGHT / TEXTURE_WIDTH * 1.0f, ft_near, ft_far);

    //Move the camera back a bit and rotate.
    translate(&view_matrix, 0, -0.0, -5.0);
    rotate(&view_matrix, -35.0, Y_AXIS);
    
    //Or use the look_at function.
    //look_at(view_matrix, eye, center, up);
    
    //Upload view and projection matrix data to GPU.
    glUniformMatrix4fv(view_matrix_uniform, 1, GL_TRUE, (float *) view_matrix);
    glUniformMatrix4fv(pojection_matrix_uniform, 1, GL_TRUE, (float *) projection_matrix);

    //Enable vsync so we can do some simple animation.
    glfwSwapInterval(1);

    printf("DONE!\n");

    init_done = GL_TRUE;

    print_help();

    //Test wireframe.
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void hw4_draw(void)
{
    int i;
    GLfloat color[4];
    //Not using any timers, just enabling vsync so ~60 fps.
    static unsigned int frame_count = 0;

    //If something failed, we can't do anything.
    if (!init_done) {
        return;
    }

    //Clear depth buffer.
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(transformation_shader->program);
    
    //Rotate the model a bit.
    memcpy(model_matrix, IDENTITY_4F, sizeof(float) * 16);
    //Different rotation speeds, ~4-8s.
    //TO-DO: There seems to be some rounding error causing very slight twitching.
    rotate(&model_matrix, (360.0f / 60 / 4) * (float) frame_count, Y_AXIS);
    rotate(&model_matrix, (360.0f / 60 / 8) * (float) frame_count, X_AXIS);
    glUniformMatrix4fv(model_matrix_uniform, 1, GL_TRUE, (float *) model_matrix);

    //Draw the object.
    glBindVertexArray(obj_vao);
    //Silly data in polygons but shouldn't matter for this homework.
    for(i=0; i < OBJ_FACE_COUNT; i++) {
        color[0] = (GLfloat) (0.11 + ((0.88 / OBJ_FACE_COUNT) * i));
        color[1] = (GLfloat) (0.44 + ((0.55 / OBJ_FACE_COUNT) * i));
        color[2] = (GLfloat) (0.44 + ((0.55 / OBJ_FACE_COUNT) * i));
        color[3] = 1.0f;
        glUniform4fv(color_uniform, 1, color);
        glDrawElements(GL_POLYGON, 5, GL_UNSIGNED_SHORT, (GLvoid *) (i * 10));
    }
    glBindVertexArray(0);

    glUseProgram(0);

    frame_count++;
    //Reset after ~16s
    if (frame_count == 60 * 16) {
        frame_count = 0;
    }

}

void hw4_terminate(void)
{
    //Restore OpenGL settings.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if(init_done) {
        //Unload shaders.
        unload_shaders(transformation_shader); 
        glUseProgram(0);
    }

    //Delete the buffers.
    glDeleteBuffers(1, &obj_vbo);
    glDeleteBuffers(1, &index_vbo);
    glDeleteVertexArrays(1, &obj_vao);

    //Disable attributes.
    glDisableVertexAttribArray(0);

    //Disable vsync.
    glfwSwapInterval(0);
}