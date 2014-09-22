// 2013/04/27 Naoyuki Hirayama

#include <cstdio>
#include <fstream>
#include "screen.hpp"
#include "mqoreader.hpp"
#include "figure.hpp"
#include "jslib.hpp"
#include "texture.hpp"
#include "partix_user.hpp"

class PartixBindings {
public:
    PartixBindings(Screen& screen) : screen_(screen) {
        world_.reset(new PartixWorld);
        world_->restart();
        global_accel_modifier_ = 1.0f;
    }

    void update() {
        softvolume_ptr v = std::dynamic_pointer_cast<softvolume_type>(body_);
        Matrix m = v->get_deformed_matrix();
        figure_->set_transform(m);
        world_->update();

        Vector view_point = screen_.make_view_point();
        if (view_point != prev_view_point_) {
            Vector up(0, 1.0f, 0);
            Vector cross0 = cross(view_point, up);
            Vector cross1 = cross(cross0, view_point);
            normalize_f(cross1);
            prev_view_point_ = view_point;

            world_->set_gravity(cross1 * -9.8f * global_accel_modifier_);
        }
    }

    void add_body(Screen& screen) {
        body_ = world_->add_entity();

        std::ifstream ifs( "data/miku2_v.mqo" );
        mqo_reader::document_type doc;
        mqo_reader::read_mqo(ifs, doc);

/*
        const Color red { {0.8, 0.1, 0.0, 1.0} };
        const Color green { {0.0, 0.8, 0.2, 1.0} };
        const Color blue { {0.2, 0.2, 1.0, 1.0} };
*/
        const Color yellow { {1.0, 1.0, 0.2, 1.0} };

        figure_.reset(new Figure);
        figure_->build_from_mqo(doc, MIKU_SCALE, yellow);

        screen.add_shape(figure_);
    }

private:
    Screen&                         screen_;
    std::unique_ptr<PartixWorld>    world_;
    body_ptr                        body_;
    figure_ptr                      figure_;
    Vector                          prev_view_point_;
    float                           global_accel_modifier_;

};

void start(int argc, const char *argv[]) {
    initMouse();
    initTextureVault();

    Screen screen(argc, argv, "yamadumi");
    std::shared_ptr<PartixBindings> bindings(new PartixBindings(screen));
    
    std::ifstream ifs( "data/miku2_v.mqo" );
    mqo_reader::document_type doc;
    mqo_reader::read_mqo(ifs, doc);

    bindings->add_body(screen);

    screen.on_idle(
        [=](float){
            bindings->update();
        });

    screen.do_main_loop();
}


int main(int argc, const char *argv[]) {
    start(argc, argv);
    return 0;
}
