#include <GL/glfw.h>
#include <GL/glext.h>

#define PI         3.14159265358979323846
#define TWO_PI     6.28318530717958647692
#define DEG_TO_RAD 0.01745329251994329576

typedef struct {
    GLint x;
    GLint y;
} Vector2i;

typedef struct {
    GLfloat x;
    GLfloat y;
} Vector2f;

typedef struct {
    GLint x;
    GLint y;
    GLint z;
} Vector3i;

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Vector3f;

typedef struct {
    GLint x;
    GLint y;
    GLint z;
    GLint w;
} Vector4i;

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
} Vector4f;

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

GLfloat INDENTITY_4F[16];

void perpective(GLfloat * matrix, GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz);
void frustum(float * matrix, float left, float right, float bottom, float top, float near_z, float far_z);
void normalize_vector(float * v);
void normal_of_plane(float * normal, const float * v1, const float * v2);
void translate2(float * matrix, float x, float y, float z);
void translate(float * matrix, float x, float y, float z);
void look_at(float * matrix, float * eyePosition3D, float * center3D, float * upVector3D);
void multiply4x4(GLfloat * result, GLfloat * m1, GLfloat * m2);
void rotate(GLfloat * matrix, GLfloat angle, AXIS axis);
