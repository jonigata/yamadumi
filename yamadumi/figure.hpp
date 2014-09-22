/*!
  @file     figure.hpp
  @brief    <ŠT—v>

  <à–¾>
  $Id: figure.hpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $
*/
#ifndef FIGURE_HPP
#define FIGURE_HPP

#include "shape.hpp"
#include "mqoreader.hpp"

class Figure : public Shape {
public:
    Figure();
    ~Figure();

    void render();
    void set_transform(const Matrix&);
    Matrix get_transform();
    Color get_material_color();

    void clear();
    bool empty();
    void build_from_mqo(
        mqo_reader::document_type&, float scale, const Color& color);
    void update(float elapsed);

private:
    std::unique_ptr<class FigureImp> imp_;

};

typedef std::shared_ptr<Figure> figure_ptr;

#endif // FIGURE_HPP
