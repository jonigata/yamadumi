// 2013/04/27 Naoyuki Hirayama

#include <cstdio>
#include "gear.hpp"

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glut.h>

#define STRIPS_PER_TOOTH 7
#define VERTICES_PER_TOOTH 34

inline void sincos(float angle, float* s, float* c) {
    *s = sin(angle);
    *c = cos(angle);
}

/*============================================================================
 *
 * class Gear 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< Gear

//****************************************************************
// constructor
Gear::Gear(float inner_radius, float outer_radius, float width,
           int teeth, float tooth_depth,
           float x, float y, const Color& color) {

    x_ = x;
    y_ = y;
    color_ = color;
    angle_ = 0;

    int cur_strip = 0;

    /* Calculate the radii used in the gear */
    float r0 = inner_radius;
    float r1 = outer_radius - tooth_depth / 2.0;
    float r2 = outer_radius + tooth_depth / 2.0;

    float da = 2.0 * M_PI / teeth / 4.0;

    /* Allocate memory for the triangle strip information */
    strips_.resize(STRIPS_PER_TOOTH * teeth);

    /* Allocate memory for the vertices */
    vertices_.resize(VERTICES_PER_TOOTH * teeth);
    Vertex* v = &vertices_[0];

    for (int i = 0; i < teeth; i++) {
        Vector normal;

        /* Calculate needed sin/cos for varius angles */
        float s[5], c[5];
        for(int j = 0 ; j < 5 ; j++) {
            sincos(i * 2.0 * M_PI / teeth + da * j, &s[j], &c[j]);
        }

        auto gear_point = [&s, &c](float r, int da) -> Vector {
            return Vector(r*c[da], r*s[da], 0);
        };

        auto start_strip = [this](int cur_strip, Vertex* v) -> void {
            strips_[cur_strip].first = v - &vertices_[0];
        };

        auto end_strip = [this](int& cur_strip, Vertex* v) -> void {
            int tmp = v - &vertices_[0];
            strips_[cur_strip].count = tmp - strips_[cur_strip].first;
            cur_strip++;
        };

        /* Create the 7 points (only x,y coords) used to draw a tooth */
        struct Vector p[7] = {
            gear_point(r2, 1), // 0
            gear_point(r2, 2), // 1
            gear_point(r1, 0), // 2
            gear_point(r1, 3), // 3
            gear_point(r0, 0), // 4
            gear_point(r1, 4), // 5
            gear_point(r0, 4), // 6
        };

        auto gear_vert =
            [width, &p, &normal](Vertex*& v, int p0, int sign) -> void {
            const Vector& pp = p[p0];
            v->p.assign(pp.x, pp.y, sign * width * 0.5);
            v->n = normal;
            v++;
        };

        auto quad_with_normal =
            [&p, &normal, &gear_vert](Vertex*& v, int p1, int p2) -> void {
            const Vector& pp1 = p[p1];
            const Vector& pp2 = p[p2];
            normal.assign(pp1.y - pp2.y, -(pp1.x - pp2.x), 0);
            gear_vert(v, p1, -1);
            gear_vert(v, p1, 1);
            gear_vert(v, p2, -1);
            gear_vert(v, p2, 1);
        };

        /* Front face */
        start_strip(cur_strip, v);
        normal.assign(0, 0, 1.0);
        gear_vert(v, 0, +1);
        gear_vert(v, 1, +1);
        gear_vert(v, 2, +1);
        gear_vert(v, 3, +1);
        gear_vert(v, 4, +1);
        gear_vert(v, 5, +1);
        gear_vert(v, 6, +1);
        end_strip(cur_strip, v);

        /* Inner face */
        start_strip(cur_strip, v);
        quad_with_normal(v, 4, 6);
        end_strip(cur_strip, v);

        /* Back face */
        start_strip(cur_strip, v);
        normal.assign(0, 0, -1.0);
        gear_vert(v, 6, -1);
        gear_vert(v, 5, -1);
        gear_vert(v, 4, -1);
        gear_vert(v, 3, -1);
        gear_vert(v, 2, -1);
        gear_vert(v, 1, -1);
        gear_vert(v, 0, -1);
        end_strip(cur_strip, v);

        /* Outer face */
        start_strip(cur_strip, v);
        quad_with_normal(v, 0, 2);
        end_strip(cur_strip, v);

        start_strip(cur_strip, v);
        quad_with_normal(v, 1, 0);
        end_strip(cur_strip, v);

        start_strip(cur_strip, v);
        quad_with_normal(v, 3, 1);
        end_strip(cur_strip, v);

        start_strip(cur_strip, v);
        quad_with_normal(v, 5, 3);
        end_strip(cur_strip, v);
    }

    /* Store the vertices in a vertex buffer object (VBO) */
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
                 &vertices_[0], GL_STATIC_DRAW);
}

//****************************************************************
// destructor
Gear::~Gear() {
}

//****************************************************************
// render
void Gear::render() {
    /* Set the vertex buffer object to use */
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    /* Set up the position of the attributes in the vertex buffer object */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), (GLfloat *) 0 + 3);

    /* Enable the attributes */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* Draw the triangle strips that comprise the gear */
    for (Strip& s : strips_) {
        glDrawArrays(GL_TRIANGLE_STRIP, s.first, s.count);
    }

    /* Disable the attributes */
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

//****************************************************************
// get_transform
Matrix Gear::get_transform() {
    Matrix m =
        Matrix::translate(x_, y_, 0) * 
        Matrix::rotate(2 * M_PI * angle_ / 360.0, 0, 0, 1);
    return m;
}

//>>>>>>>>>> Gear

