// 2014/09/20 Naoyuki Hirayama

#ifndef BG_HPP_
#define BG_HPP_

#include "piece.hpp"

class BG {
public:
    BG() {
        piece_.vbo = 0;
        piece_.ibo = 0;
        piece_.texture = "city_night002.png";
        piece_.color.rgba[0] = 1.0;
        piece_.color.rgba[1] = 1.0;
        piece_.color.rgba[2] = 1.0;
        piece_.color.rgba[3] = 1.0;

        add_vertex(0, 0);
        add_vertex(1, 0);
        add_vertex(0, 1);
        add_vertex(1, 1);
        piece_.index_source.push_back(0);
        piece_.index_source.push_back(1);
        piece_.index_source.push_back(2);
        piece_.index_source.push_back(2);
        piece_.index_source.push_back(1);
        piece_.index_source.push_back(3);

        piece_.build();
    }

    void add_vertex(float x, float y){
        Piece::Vertex dst;
        dst.position = Vector(x, y, 0);
        dst.normal = Vector(0, 0, 1);
        dst.diffuse = piece_.color;
        dst.u = x;
        dst.v = 1 - y;
        piece_.vertex_source.push_back(dst);
    }

    void render() {
        piece_.render();
    }
        

private:
    Piece piece_;
};

#endif // BG_HPP_
