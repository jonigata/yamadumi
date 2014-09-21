.SUFFIXES : .cpp .o .js

OBJS = yamadumi.o screen.o mqoreader.o figure.o camera.o mouse_dispatcher.o camera.o
JSS = yamadumi_lib.js 

yamadumi.js: $(OBJS) $(JSS)
	emcc -std=c++11 --js-library $(JSS) $(OBJS) -o yamadumi.js --embed-file data -s EXPORTED_FUNCTIONS="['_main','_malloc','_addMouseEvent']"

.cpp.o:
	emcc -std=c++11 $< -o $@

screen.o : screen.cpp bg.hpp

clean:
	rm -f $(OBJS) yamadumi.js



