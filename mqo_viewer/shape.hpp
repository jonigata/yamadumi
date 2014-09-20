// 2013/04/29 Naoyuki Hirayama

/*!
	@file	  shape.hpp
	@brief	  <概要>

	<説明>
*/

#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <memory>
#include "geometry.hpp"

struct Color {
    float rgba[4];
};

class Shape {
public:

public:
    virtual ~Shape(){}

    virtual void render() = 0;

    virtual Matrix get_transform() = 0;
    virtual Color get_material_color() = 0;
};

typedef std::shared_ptr<Shape> shape_ptr;

#endif // SHAPE_HPP_
