// 2013/04/27 Naoyuki Hirayama

#include <cstdio>
#include <fstream>
#include "screen.hpp"
#include "mqoreader.hpp"
#include "figure.hpp"
#include "jslib.hpp"
#include "texture.hpp"

const float MIKU_SCALE = 0.02f;

void start(int argc, const char *argv[]) {
    initMouse();
    initTextureVault();

    Screen screen(argc, argv, "yamadumi");
    
    const Color red { {0.8, 0.1, 0.0, 1.0} };
    const Color green { {0.0, 0.8, 0.2, 1.0} };
    const Color blue { {0.2, 0.2, 1.0, 1.0} };
    const Color yellow { {1.0, 1.0, 0.2, 1.0} };
    
    std::ifstream ifs( "data/miku2_v.mqo" );
    mqo_reader::document_type doc;
    mqo_reader::read_mqo(ifs, doc);

    figure_ptr figure(new Figure);
    figure->build_from_mqo(doc, MIKU_SCALE, yellow);

    screen.add_shape(figure);

                
    screen.on_idle(
        [](float){
        });

    screen.do_main_loop();
}


int main(int argc, const char *argv[]) {
    start(argc, argv);
    return 0;
}
