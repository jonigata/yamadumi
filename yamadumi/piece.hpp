// 2014/09/20 Naoyuki Hirayama

#ifndef PIECE_HPP_
#define PIECE_HPP_

#include "geometry.hpp"
#include "color.hpp"
#include "gl.hpp"
#include "texture.hpp"
#include <vector>
#include <string>

struct Piece {
    struct Vertex {
        Vector      position;
        Vector      normal;
        Color       diffuse;
        float       u;
        float       v;
        Vertex() {}
        Vertex(const Vector& p, const Vector& n, const Color& c) {
            position = p;
            normal = n;
            diffuse = c;
        }
    };

    typedef uint16_t Index;

    std::vector<Vertex>     vertex_source;
    std::vector<Index>      index_source;
    GLuint                  vbo;
    GLuint                  ibo;
    std::string             texture;
    Color                   color;

    void build() {
        loadTexture(texture.c_str());

        glGenBuffers(1, &this->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     this->vertex_source.size() * sizeof(Vertex),
                     &this->vertex_source[0],
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &this->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     this->index_source.size() * sizeof(Index),
                     &this->index_source[0],
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void render() {
        if (this->vbo ==0 || this->ibo == 0) {
            //printf("no contents\n");
            return ;
        }

        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);

        /* Set up the position of the attributes in the vertex buffer object */
        int stride = sizeof(Vertex);
        const float* p = 0;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, p + 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, p + 3);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, p + 6);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, p + 10);

        /* Enable the attributes */
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        bindTexture(this->texture.c_str());

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElements(
            GL_TRIANGLES,
            this->index_source.size(),
            GL_UNSIGNED_SHORT,
            nullptr);

        int error = glGetError();
        if (error != 0) {
            printf("error: %d\n", error);
        }

        /* Disable the attributes */
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};

#endif // PIECE_HPP_
