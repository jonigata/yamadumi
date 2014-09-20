// 2013/04/27 Naoyuki Hirayama

#ifndef GEAR_HPP_
#define GEAR_HPP_

#include <vector>
#include <array>
#include "shape.hpp"

class Gear : public Shape {
public:
    Gear(float inner_radius, float outer_radius, float width,
         int teeth, float tooth_depth,
         float x, float y, const Color& color);
    ~Gear();

    Matrix transform();
    void render();

    void set_angle(float a) { angle_ = a; }

    Matrix get_transform();
    Color get_material_color() { return color_; }
    
private:
    struct Vertex {
        Vector  p;
        Vector  n;
        Vertex() {}
        Vertex(const Vector& ap, const Vector& an) { p = ap; n = an; }
    };

    struct Strip {
        unsigned int first;
        unsigned int count;
    };

    std::vector<Vertex> vertices_;
    std::vector<Strip>  strips_;
    unsigned int        vbo_;

    float x_;
    float y_;
    float angle_;
    Color color_;
    
};

typedef std::shared_ptr<Gear> gear_ptr;

#endif // GEAR_HPP_

