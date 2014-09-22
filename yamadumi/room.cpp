// $Id: room.cpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $

#include "room.hpp"
#include <cstdio>

/*============================================================================
 *
 * class Room 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< Room

//****************************************************************
// constructor
Room::Room()
{
    Color c;
    c.rgba[0] = 0.2;
    c.rgba[1] = 0.2;
    c.rgba[2] = 0.9;
    c.rgba[3] = 0.25;

    Vertex face_vertices[8];
    face_vertices[0](-5.0f, -5.0f, -5.0f, c);
    face_vertices[1]( 5.0f, -5.0f, -5.0f, c);
    face_vertices[2]( 5.0f,  5.0f, -5.0f, c);
    face_vertices[3](-5.0f,  5.0f, -5.0f, c);
    face_vertices[4](-5.0f,  5.0f,  5.0f, c);
    face_vertices[5]( 5.0f,  5.0f,  5.0f, c);
    face_vertices[6]( 5.0f, -5.0f,  5.0f, c);
    face_vertices[7](-5.0f, -5.0f,  5.0f, c);

    const static uint16_t face_indices[] = {
        0, 1, 2,  0, 2, 3,
        2, 4, 3,  2, 5, 4,
        4, 5, 6,  4, 6, 7,
        6, 0, 7,  6, 1, 0,
        0, 3, 4,  0, 4, 7,
        1, 5, 2,  1, 6, 5,
    };
        
    c.rgba[0] = 0.9;
    c.rgba[1] = 0.9;
    c.rgba[2] = 0.7;
    c.rgba[3] = 0.7;

/*
    Vertex edge_vertices[8];
    edge_vertices[0](-5.0f, -5.0f, -5.0f, c);
    edge_vertices[1]( 5.0f, -5.0f, -5.0f, c);
    edge_vertices[2]( 5.0f,  5.0f, -5.0f, c);
    edge_vertices[3](-5.0f,  5.0f, -5.0f, c);
    edge_vertices[4](-5.0f,  5.0f,  5.0f, c);
    edge_vertices[5]( 5.0f,  5.0f,  5.0f, c);
    edge_vertices[6]( 5.0f, -5.0f,  5.0f, c);
    edge_vertices[7](-5.0f, -5.0f,  5.0f, c);

    const static uint16_t edge_indices[24] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,

        4, 5,
        5, 6,
        6, 7,
        7, 4,

        0, 7,
        1, 6,
        2, 5,
        3, 4
    };
*/
        
    glGenBuffers(1, &this->face_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->face_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(face_vertices), &face_vertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &this->face_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->face_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_indices), &face_indices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//****************************************************************
// destructor
Room::~Room()
{
}

//****************************************************************
// render
void Room::render() {
    if (this->face_vbo ==0 || this->face_ibo == 0) {
        //printf("no contents\n");
        return ;
    }

    glBindBuffer(GL_ARRAY_BUFFER, this->face_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->face_ibo);

    /* Set up the position of the attributes in the vertex buffer object */
    int stride = sizeof(Vertex);
    const float* p = 0;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, stride, p + 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, stride, p + 3);

    /* Enable the attributes */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDrawElements(
        GL_TRIANGLES,
        36,
        GL_UNSIGNED_SHORT,
        nullptr);

    int error = glGetError();
    if (error != 0) {
        printf("error: %d\n", error);
    }

    /* Disable the attributes */
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//>>>>>>>>>> Room

