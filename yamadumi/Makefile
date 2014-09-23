.SUFFIXES : .cpp .o .js

OBJS = yamadumi.o screen.o mqoreader.o figure.o camera.o mouse_dispatcher.o camera.o room.o
JSS = yamadumi_lib.js 

yamadumi.js: $(OBJS) $(JSS)
	emcc -std=c++11 --js-library $(JSS) $(OBJS) -o yamadumi.js --embed-file data -s EXPORTED_FUNCTIONS="['_main','_malloc','_addMouseEvent','_addSliderEvent']" -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1

.cpp.o:
	emcc -O3 -std=c++11 -Wall -Werror $< -o $@

screen.o : screen.cpp bg.hpp shader.hpp

yamadumi.o : partix_user.hpp

clean:
	rm -f $(OBJS) yamadumi.js



