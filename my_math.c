#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "my_math.h"

GLfloat INDENTITY_4F[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void perpective(GLfloat * matrix, GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz)
{
    float ymax, xmax;
    ymax = nearz * tanf(fov * PI / 360.0);
    xmax = ymax * aspect;
    frustum(matrix, -xmax, xmax, -ymax, ymax, nearz, farz);
}

void frustum(float *matrix, float left, float right, float bottom, float top, float near_z, float far_z)
{
    float temp, temp2, temp3, temp4;
    temp = (GLfloat) 2.0 * near_z;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = far_z - near_z;
    matrix[0] = temp / temp2;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;
    matrix[4] = 0.0;
    matrix[5] = temp / temp3;
    matrix[6] = 0.0;
    matrix[7] = 0.0;
    matrix[8] = (right + left) / temp2;
    matrix[9] = (top + bottom) / temp3;
    matrix[10] = (-far_z - near_z) / temp4;
    matrix[11] = -1.0;
    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = (-temp * far_z) / temp4;
    matrix[15] = 0.0;
}

void normalize_vector(float * v)
{
    double len;
    len = sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));

    if(!len)
        return;

    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

void normal_of_plane(float * normal, const float * v1, const float * v2)
{
    normal[0]=(v1[1] * v2[2]) - (v1[2] * v2[1]);
    normal[1]=(v1[2] * v2[0]) - (v1[0] * v2[2]);
    normal[2]=(v1[0] * v2[1]) - (v1[1] * v2[0]);
}

void translate2(float * matrix, float x, float y, float z)
{
    matrix[12]=matrix[0]*x+matrix[4]*y+matrix[8]*z+matrix[12];
    matrix[13]=matrix[1]*x+matrix[5]*y+matrix[9]*z+matrix[13];
    matrix[14]=matrix[2]*x+matrix[6]*y+matrix[10]*z+matrix[14];
    matrix[15]=matrix[3]*x+matrix[7]*y+matrix[11]*z+matrix[15];
}

void look_at(float * matrix, float * eye, float * center, float * up_vector)
{
    float forward[3], side[3], up[3];
    float matrix2[16], resultMatrix[16];

    forward[0] = center[0] - eye[0];
    forward[1] = center[1] - eye[1];
    forward[2] = center[2] - eye[2];
    normalize_vector(forward);

    normal_of_plane(side, forward, up_vector);
    normalize_vector(side);

    normal_of_plane(up, side, forward);

    matrix2[0] = side[0];
    matrix2[4] = side[1];
    matrix2[8] = side[2];
    matrix2[12] = 0.0;

    matrix2[1] = up[0];
    matrix2[5] = up[1];
    matrix2[9] = up[2];
    matrix2[13] = 0.0;

    matrix2[2] = -forward[0];
    matrix2[6] = -forward[1];
    matrix2[10] = -forward[2];
    matrix2[14] = 0.0;

    matrix2[3] = matrix2[7] = matrix2[11] = 0.0;
    matrix2[15] = 1.0;

    multiply4x4(resultMatrix, matrix, matrix2);
    translate2(resultMatrix,
        -eye[0], -eye[1], -eye[2]);

    memcpy(matrix, resultMatrix, 16*sizeof(float));
}

void multiply4x4(GLfloat * result, GLfloat * m1, GLfloat * m2)
{
    GLfloat temp[16];
    int x,y;

    memcpy(temp, INDENTITY_4F, sizeof(GLfloat) * 16);

    for (x=0; x < 4; x++)
    {
        for(y=0; y < 4; y++)
        {
            temp[y + (x*4)] = (m1[x*4] * m2[y]) +
                (m1[(x*4)+1] * m2[y+4]) +
                (m1[(x*4)+2] * m2[y+8]) +
                (m1[(x*4)+3] * m2[y+12]);
        }
    }

    memcpy(result, temp, sizeof(GLfloat) * 16);
}

void translate(GLfloat * matrix, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat new_matrix[16];
    memcpy(new_matrix, INDENTITY_4F, sizeof(GLfloat) * 16);

    new_matrix[12] = x;
    new_matrix[13] = y;
    new_matrix[14] = z;

    multiply4x4(matrix, matrix, new_matrix);
}

void rotate(GLfloat * matrix, GLfloat angle, AXIS axis)
{
    const int cos1[3] = { 5, 0, 0 };
    const int cos2[3] = { 10, 10, 5 };
    const int sin1[3] = { 6, 2, 1 };
    const int sin2[3] = { 9, 8, 4 };

    GLfloat new_matrix[16];
    memcpy(new_matrix, INDENTITY_4F, sizeof(GLfloat) * 16);

    new_matrix[cos1[axis]] = (GLfloat) cos(DEG_TO_RAD * angle);
    new_matrix[sin1[axis]] = (GLfloat) -sin(DEG_TO_RAD * angle);
    new_matrix[sin2[axis]] = -new_matrix[sin1[axis]];
    new_matrix[cos2[axis]] = new_matrix[cos1[axis]];

    multiply4x4(matrix, matrix, new_matrix);
}