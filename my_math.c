#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "my_math.h"

Matrix2f IDENTITY_2F = {
    {1.0f, 0.0f},
    {0.0f, 1.0f}
};

Matrix3f IDENTITY_3F = {
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f}
};

Matrix4f IDENTITY_4F = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f}
};

float inverse_sqrt(float x)
{
    //Newton–Raphson method to calculate inverse square using magical first guess.
    //http://www.codemaestro.com/reviews/9
    float x_half = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f375a86 - (i >> 1);
    x = * (float*) &i;
    //Two Newton steps. Could use one for 1e-3 precision.
    x = x * (1.5f - x_half * x * x);
    x = x * (1.5f - x_half * x * x);
    return x;
}

void transponse2f(const Matrix2f * matrix, Matrix2f * result)
{
    (*result)[0][0] = (*matrix)[0][0];
    (*result)[0][1] = (*matrix)[1][0];

    (*result)[1][0] = (*matrix)[0][1];
    (*result)[1][1] = (*matrix)[1][1];
}

void transponse2f_self(Matrix2f * matrix)
{
    float tmp;

    tmp = *matrix[0][1];
    *matrix[0][1] = *matrix[1][0];
    *matrix[1][0] = tmp;
}

void transponse3f(const Matrix3f * matrix, Matrix3f * result)
{
    *result[0][0] = *matrix[0][0];
    *result[0][1] = *matrix[1][0];
    *result[0][2] = *matrix[2][0];

    *result[1][0] = *matrix[0][1];
    *result[1][1] = *matrix[1][1];
    *result[1][2] = *matrix[2][1];

    *result[2][0] = *matrix[0][2];
    *result[2][1] = *matrix[1][2];
    *result[2][2] = *matrix[2][2];
}

void transponse3f_self(Matrix3f * matrix)
{
    float temp;

    temp = *matrix[0][1];
    (*matrix)[0][1] = (*matrix)[1][0];
    (*matrix)[1][0] = temp;

    temp = *matrix[0][2];
    (*matrix)[0][2] = (*matrix)[2][0];
    (*matrix)[2][0] = temp;

    temp = *matrix[1][2];
    (*matrix)[1][2] = (*matrix)[2][1];
    (*matrix)[2][1] = temp;
}

void transponse4f(const Matrix2f * matrix, Matrix2f * result)
{
    int i, j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            (*result)[i][j] = (*matrix)[j][i];
        }
    }
}

void transponse4f_self(Matrix2f * matrix)
{
    float temp;
    int i, j;

    for(i = 0; i < 4; i++) {
        for(j = i + 1; j < 4; j++) {
            temp = *matrix[i][j];
            *matrix[i][j] = *matrix[j][i];
            *matrix[j][i] = temp;
        }
    }
}

float vector2f_normalize(Vector2f * v)
{
    float sq_len;
    float inv_len;

    sq_len = (v->x * v->x) + (v->y * v->y) ;
    inv_len = inverse_sqrt(sq_len);

    v->x *= inv_len;
    v->y *= inv_len;

    return inv_len * sq_len;
}

float vector3f_normalize(Vector3f * v)
{
    float sq_len;
    float inv_len;

    sq_len = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
    inv_len = inverse_sqrt(sq_len);

    v->x *= inv_len;
    v->y *= inv_len;
    v->z *= inv_len;

    return inv_len * sq_len;
}

float vector4f_normalize(Vector4f * v)
{
    float sq_len;
    float inv_len;

    sq_len = (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
    inv_len = inverse_sqrt(sq_len);

    v->x *= inv_len;
    v->y *= inv_len;
    v->z *= inv_len;
    v->w *= inv_len;

    return inv_len * sq_len;
}

void vector3f_coss(Vector3f * normal, const Vector3f * v1, const Vector3f * v2)
{
    normal->x = (v1->y * v2->z) - (v1->z * v2->y);
    normal->y = (v1->z * v2->x) - (v1->x * v2->z);
    normal->z = (v1->x * v2->y) - (v1->y * v2->x);
}

void orthographic(Matrix4f * matrix, float width, float height, float z_near, float z_far)
{
    (*matrix)[0][0] = 2 / width;
    (*matrix)[0][1] = 0;
    (*matrix)[0][2] = 0;
    (*matrix)[0][3] = 0;

    (*matrix)[1][0] = 0;
    (*matrix)[1][1] = 2 / height;
    (*matrix)[1][2] = 0;
    (*matrix)[1][3] = 0;

    (*matrix)[2][0] = 0;
    (*matrix)[2][1] = 0;
    (*matrix)[2][2] = -2.0f / (z_far - z_near);
    (*matrix)[2][3] = (-z_far - z_near) / (z_far - z_near);

    (*matrix)[3][0] = 0;
    (*matrix)[3][1] = 0;
    (*matrix)[3][2] = 0;
    (*matrix)[3][3] = 1;
}


void perspective(Matrix4f * matrix, float fov, float aspect, float z_near, float z_far)
{
    float y_max;
    float x_max;
    float width;
    float height;

    y_max = z_near * tanf(fov * PI / 360.0);
    x_max = y_max * aspect;

    width = 2 * x_max;
    height = 2 * y_max;

    (*matrix)[0][0] = (2.0f * z_near) / width;
    (*matrix)[0][1] = 0;
    (*matrix)[0][2] = 0; //(right + left) / (right - left), in our case 0.
    (*matrix)[0][3] = 0;

    (*matrix)[1][0] = 0;
    (*matrix)[1][1] = (2.0f * z_near) / height;
    (*matrix)[1][2] = 0; //(top + bottom) / (top - bottom), in our case 0.
    (*matrix)[1][3] = 0;

    (*matrix)[2][0] = 0;
    (*matrix)[2][1] = 0;
    (*matrix)[2][2] = (-z_far - z_near) / (z_far - z_near);
    (*matrix)[2][3] = (-2 * z_far * z_near) / (z_far - z_near);

    (*matrix)[3][0] = 0;
    (*matrix)[3][1] = 0;
    (*matrix)[3][2] = -1.0f;
    (*matrix)[3][3] = 0;
}

void perspective_infinite(Matrix4f * matrix, float fov, float aspect, float z_near)
{
    float y_max;
    float x_max;
    float width;
    float height;

    y_max = z_near * tanf(fov * PI / 360.0);
    x_max = y_max * aspect;

    width = 2 * x_max;
    height = 2 * y_max;

    (*matrix)[0][0] = 2 * z_near / width;
    (*matrix)[0][1] = 0;
    (*matrix)[0][2] = 0; //(right + left) / (right - left), in our case 0.
    (*matrix)[0][3] = 0;

    (*matrix)[1][0] = 0;
    (*matrix)[1][1] = 2 * z_near / height;
    (*matrix)[1][2] = 0; //(top + bottom) / (top - bottom), in our case 0.
    (*matrix)[1][3] = 0;

    (*matrix)[2][0] = 0;
    (*matrix)[2][1] = 0;
    (*matrix)[2][2] = -1;
    (*matrix)[2][3] = -2 * z_near;

    (*matrix)[3][0] = 0;
    (*matrix)[3][1] = 0;
    (*matrix)[3][2] = -1;
    (*matrix)[3][3] = 0;
}

void multiply4f(Matrix4f * result, const Matrix4f * m1, const Matrix4f * m2)
{
    int i, j;
    const float *m1_pointer, *m2_pointer;
    Matrix4f result_matrix;
    float *result_pointer;

    m1_pointer = (const float *) m1;
    m2_pointer = (const float *) m2;
    result_pointer = (float *) &result_matrix;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            *result_pointer = m1_pointer[0] * m2_pointer[0 * 4 + j]
                            + m1_pointer[1] * m2_pointer[1 * 4 + j]
                            + m1_pointer[2] * m2_pointer[2 * 4 + j]
                            + m1_pointer[3] * m2_pointer[3 * 4 + j];
            result_pointer++;
        }
        m1_pointer += 4;
    }
    memcpy(result, result_matrix, sizeof(Matrix4f));
}

void translate(Matrix4f * matrix, float x, float y, float z)
{
    (*matrix)[0][3] += ((*matrix)[0][0] * x) + ((*matrix)[0][1] * y) + ((*matrix)[0][2] * z);
    (*matrix)[1][3] += ((*matrix)[1][0] * x) + ((*matrix)[1][1] * y) + ((*matrix)[1][2] * z);
    (*matrix)[2][3] += ((*matrix)[2][0] * x) + ((*matrix)[2][1] * y) + ((*matrix)[2][2] * z);
    (*matrix)[3][3] += ((*matrix)[3][0] * x) + ((*matrix)[3][1] * y) + ((*matrix)[3][2] * z);
}

void rotate(Matrix4f * matrix, float angle, AXIS axis)
{
    //const char cos1[3][2] = {{1, 1}, {0, 0}, {0, 0}};
    //const char sin2[3][2] = {{1, 2}, {2, 0}, {0, 1}};
    //const char cos2[3][2] = {{2, 1}, {0, 2}, {1, 0}};
    //const char sin1[3][2] = {{2, 2}, {2, 2}, {1, 1}};

    const int cos1[3] = { 5, 0, 0 };
    const int cos2[3] = { 10, 10, 5 };
    const int sin1[3] = { 9, 8, 4 };
    const int sin2[3] = { 6, 2, 1 };

    Matrix4f new_matrix;
    float * m1_pointer = (float *) &new_matrix;

    memcpy(new_matrix, IDENTITY_4F, sizeof(Matrix4f));


    m1_pointer[cos1[axis]] = (float) cos(DEG_TO_RAD * angle);
    m1_pointer[sin1[axis]] = (float) -sin(DEG_TO_RAD * angle);
    m1_pointer[sin2[axis]] = -m1_pointer[sin1[axis]];
    m1_pointer[cos2[axis]] = m1_pointer[cos1[axis]];


    //new_matrix[cos1[axis][0]][cos1[axis][1]] = (float) cos(DEG_TO_RAD * angle);
    //new_matrix[sin1[axis][0]][sin1[axis][1]] = (float) -sin(DEG_TO_RAD * angle);
    //new_matrix[sin2[axis][0]][sin2[axis][1]] = -new_matrix[sin1[axis][0]][sin1[axis][1]];
    //new_matrix[cos2[axis][0]][cos2[axis][1]] = new_matrix[cos1[axis][0]][cos1[axis][1]];

    //memcpy(matrix, new_matrix, sizeof(Matrix4f));
    multiply4f(matrix, matrix, &new_matrix);
}







//void look_at(float * matrix, float * eye, float * center, float * up_vector)
//{
//    float forward[3], side[3], up[3];
//    float matrix2[16], resultMatrix[16];
//
//    forward[0] = center[0] - eye[0];
//    forward[1] = center[1] - eye[1];
//    forward[2] = center[2] - eye[2];
//    normalize_vector(forward);
//
//    normal_of_plane(side, forward, up_vector);
//    normalize_vector(side);
//
//    normal_of_plane(up, side, forward);
//
//    matrix2[0] = side[0];
//    matrix2[4] = side[1];
//    matrix2[8] = side[2];
//    matrix2[12] = 0.0;
//
//    matrix2[1] = up[0];
//    matrix2[5] = up[1];
//    matrix2[9] = up[2];
//    matrix2[13] = 0.0;
//
//    matrix2[2] = -forward[0];
//    matrix2[6] = -forward[1];
//    matrix2[10] = -forward[2];
//    matrix2[14] = 0.0;
//
//    matrix2[3] = matrix2[7] = matrix2[11] = 0.0;
//    matrix2[15] = 1.0;
//
//    multiply4x4(resultMatrix, matrix, matrix2);
//    translate2(resultMatrix,
//        -eye[0], -eye[1], -eye[2]);
//
//    memcpy(matrix, resultMatrix, 16*sizeof(float));
//}
