#include <stdio.h>
#include <stdlib.h>

#include <GL/glfw.h>

#include "hw3.h"
#include "common.h"

//Max points the polygon can contain. Can use pretty much any sane number here.
#define MAX_CORNERS 20

static Vector2i * polygon[MAX_CORNERS];
static GLint points;
static GLint last_x, last_y;
static GLint building;

static void add_point(GLint x, GLint y)
{
    polygon[points] = (Vector2i *) malloc(sizeof(Vector2i));
    polygon[points]->x = x;
    polygon[points]->y = y;

    points++;
}

static void delete_polygon(void)
{
    GLint i;
    for (i = 0; i < MAX_CORNERS; i++) {
        if (polygon[i]) {
            free(polygon[i]);
            polygon[i] = NULL;
        }
    }
    points = 0;
}

static void fill_poligon(void)
{
    GLint y_line, node_count, nodes[MAX_CORNERS], i, j;

    //Clear the image.
    bitmap_fill(&texture_data[0], 255, 255, 255);

    //  Loop through the rows of the image.
    for (y_line=0; y_line < TEXTURE_HEIGHT; y_line++) {

        node_count = 0;
        j = points - 1;

        //Put intersections in array.
        for (i = 0; i < points; i++) {
            if (polygon[i]->y < y_line && polygon[j]->y >= y_line || polygon[j]->y < y_line && polygon[i]->y >= y_line) {
                nodes[node_count++] = (GLint) (polygon[i]->x + (double)(y_line - polygon[i]->y) / (double) (polygon[j]->y - polygon[i]->y) * (double) (polygon[j]->x - polygon[i]->x));
            }
            j = i;
        }

        //Bubble sort.
        i=0;
        while (i < node_count - 1) {
            if (nodes[i]>nodes[i+1]) {
                //Swap.
                j = nodes[i];
                nodes[i] = nodes[i+1];
                nodes[i+1] = j;
                if (i)
                    i--; 
            } else {
                i++; 
            }
        }

        //Fill the pixels between node pairs.
        for (i = 0; i < node_count; i += 2) {
            if (nodes[i] >= TEXTURE_WIDTH) break;

            if (nodes[i+1]> 0) {
                if (nodes[i] < 0 )
                    nodes[i]=0 ;

                if (nodes[i+1] > TEXTURE_WIDTH)
                    nodes[i+1] = TEXTURE_WIDTH;

                for (j=nodes[i]; j < nodes[i+1]; j++)
                    bitmap_set_pixel(&texture_data[0], j, y_line, 128, 0, 0); 
            }
        }
    }
}

//Mouse click handler for this particular homework.
static void GLFWCALL click_handler(GLint button, GLint action)
{
    GLint x, y;
    //Left click, adding a new point.
    if (action == GLFW_PRESS) {
        glfwGetMousePos(&x, &y);

        printf("<click button=%x x=%d y=%d>\n", button, x, y);

        //If left click, add point to the polygon.
        if (button == 0) {
            if (points >= MAX_CORNERS && building) {
                play_error_sound();
                printf("Error: Trying to make larger polygon than currently supported. Current max point count is: %d\n", MAX_CORNERS);
            }

            if (!building) {
                //We're building a new polygon so we need to clear the screen and delete the old polygon.
                bitmap_fill(&texture_data[0], 255, 255, 255);
                delete_polygon();
                building = GL_TRUE;
                add_point(x, y);
            } else {
                add_point(x, y);
                //Draws a line to help understanding how the polygon will look like.
                bitmap_draw_line(&texture_data[0], last_x, last_y, x, y, 255, 0 , 0);

            }

            last_x = x;
            last_y = y;

            //Draw a small circle around the point to help spotting it.
            bitmap_draw_circle(&texture_data[0], x, y, 5, 0, 0, 0);
        } else if (button == 1) {
            //Right click, if we were building polygon we are done now and we just need to fill it.
            if(building) {
                building = GL_FALSE;
                bitmap_draw_line(&texture_data[0], polygon[0]->x, polygon[0]->y, last_x, last_y, 255, 0 , 0);
                fill_poligon();
            }
        }
    }
    //Update the texture.
    bitmap_update(&texture_data[0], textures[0]);
}

void hw3_init(void)
{
    printf("Initializing homework 3 ...\t");

    //Initialize defaults.
    points = 0;
    building = GL_FALSE;

    glfwSetWindowTitle("GFX Homework: 3.d");
    //Attack our click handler.
    glfwSetMouseButtonCallback(click_handler);

    //Color it white.
    bitmap_fill(&texture_data[0], 255, 255, 255);
    bitmap_upload(&texture_data[0], textures[0]);

    printf("DONE!\n");
}

void hw3_draw(void)
{
    //Draw quad and texture it using our bitmap.
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
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

    glDisable(GL_TEXTURE_2D);
}

void hw3_terminate(void)
{
    printf("Terminating homework 3 ...\t");
    //Remove callback and free up memory.
    glfwSetMouseButtonCallback(NULL);
    delete_polygon();
    printf("DONE!\n");
}