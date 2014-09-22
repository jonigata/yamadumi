/*!
  @file     room.hpp
  @brief    <ŠT—v>

  <à–¾>
  $Id: room.hpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $
*/
#ifndef ROOM_HPP
#define ROOM_HPP

#include "geometry.hpp"
#include "color.hpp"
#include "gl.hpp"

class Room {
private:
    struct Vertex {
        Vector  pos;
        Color   color;

        void operator()(float xx,float yy,float zz,const Color& cc) {
            pos.x = xx; pos.y = yy; pos.z = zz; color = cc;
        }
    };

public:
    Room();
    ~Room();

    void render();

private:
    GLuint face_vbo;
    GLuint face_ibo;
/*
    GLuint edge_vbo;
    GLuint edge_ibo;
*/

};

#endif // ROOM_HPP
