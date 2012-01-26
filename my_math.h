#include <GL/glfw.h>
#include <GL/glext.h>

#define PI         3.14159265358979323846
#define TWO_PI     6.28318530717958647692
#define DEG_TO_RAD 0.01745329251994329576
#define RAD_TO_DEG 57.2957795130823208767

typedef struct {
    int x;
    int y;
} Vector2i;

typedef struct {
    float x;
    float y;
} Vector2f;

typedef struct {
    int x;
    int y;
    int z;
} Vector3i;

typedef struct {
    float x;
    float y;
    float z;
} Vector3f;

typedef struct {
    int x;
    int y;
    int z;
    int w;
} Vector4i;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vector4f;

typedef float Matrix2f[2][2];
typedef float Matrix3f[3][3];
typedef float Matrix4f[4][4];

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

Matrix2f IDENTITY_2F;
Matrix3f IDENTITY_3F;
Matrix4f IDENTITY_4F;

float inverse_sqrt(float x);

void transponse2f(const Matrix2f * matrix, Matrix2f * result);
void transponse2f_self(Matrix2f * matrix);
void transponse3f(const Matrix3f * matrix, Matrix3f * result);
void transponse3f_self(Matrix3f * matrix);
void transponse4f(const Matrix2f * matrix, Matrix2f * result);
void transponse4f_self(Matrix2f * matrix);

float vector2f_normalize(Vector2f * v);
float vector3f_normalize(Vector3f * v);
float vector4f_normalize(Vector4f * v);

void vector3f_coss(Vector3f * normal, const Vector3f * v1, const Vector3f * v2);

void orthographic(Matrix4f * matrix, float width, float height, float z_near, float z_far);
void perspective(Matrix4f * matrix, float fov, float aspect, float z_near, float z_far);
void perspective_infinite(Matrix4f * matrix, float fov, float aspect, float z_near);

void multiply4f(Matrix4f * result, const Matrix4f * m1, const Matrix4f * m2);

void translate(Matrix4f * matrix, float x, float y, float z);
void rotate(Matrix4f * matrix, float angle, AXIS axis);
