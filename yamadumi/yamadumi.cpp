// 2013/04/27 Naoyuki Hirayama

#include <cstdio>
#include <fstream>
#include "screen.hpp"
#include "mqoreader.hpp"
#include "figure.hpp"
#include "jslib.hpp"
#include "texture.hpp"
#include "partix_user.hpp"
#include "gl.hpp"

class PartixBindings {
public:
    PartixBindings(Screen& screen) : screen_(screen) {
        world_.reset(new PartixWorld);
        world_->restart();
        global_accel_modifier_ = 1.0f;

        std::ifstream ifs( "data/miku2_v.mqo" );
        mqo_reader::read_mqo(ifs, doc_);

    }

    void update() {
        world_->update();

        for (const auto& bind: binds_) {
            update_entity(bind.first, bind.second);
        }
        
        Vector view_point = screen_.make_view_point();
        if (view_point != prev_view_point_) {
            Vector up(0, 1.0f, 0);
            Vector cross0 = cross(view_point, up);
            Vector cross1 = cross(cross0, view_point);
            normalize_f(cross1);
            prev_view_point_ = view_point;

            world_->set_gravity(cross1 * -9.8f * 1.0 * global_accel_modifier_);
        }
    }

    void update_entity(body_ptr body, figure_ptr figure) {
        softvolume_ptr v = std::dynamic_pointer_cast<softvolume_type>(body);
        Matrix m = v->get_deformed_matrix();
        figure->set_transform(m);
    }

    void add_body(Screen& screen) {
        body_ptr body = world_->add_entity();

/*
        const Color red { {0.8, 0.1, 0.0, 1.0} };
        const Color green { {0.0, 0.8, 0.2, 1.0} };
        const Color blue { {0.2, 0.2, 1.0, 1.0} };
*/
        const Color yellow { {1.0, 1.0, 0.2, 1.0} };

        figure_ptr figure(new Figure);
        figure->build_from_mqo(doc_, MIKU_SCALE, yellow);

        screen.add_shape(figure);

        binds_.push_back(std::make_pair(body, figure));

        printf("figure count: %d\n", binds_.size());
    }

    void slider(int which, float value) {
        switch(which) {
            case 0:
                world_->set_stretch_factor(value);
                break;
            case 1:
                world_->set_restore_factor(value);
                break;
            case 2:
                world_->set_friction(value);
                break;
        }
    }

private:
    Screen&                         screen_;

    mqo_reader::document_type       doc_;
    std::unique_ptr<PartixWorld>    world_;
    Vector                          prev_view_point_;
    float                           global_accel_modifier_;
    std::vector<std::pair<body_ptr, figure_ptr>> binds_;

};

void start(int argc, const char *argv[]) {
    initMouse();
    initSliders();
    initTextureVault();

    Screen screen(argc, argv, "yamadumi");
    std::shared_ptr<PartixBindings> bindings(new PartixBindings(screen));

    screen.on_slider(
        [&](int which, float value) {
            bindings->slider(which, value);
        });

    screen.on_keyboard(
        [&](int code) {
            printf("%d\n", code);
            switch (code) {
                case ' ':
                    bindings->add_body(screen);
                    break;
            }
        });
    
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
