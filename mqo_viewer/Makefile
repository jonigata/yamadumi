.SUFFIXES : .cpp .o .js

mqo_viewer.js: mqo_viewer.o screen.o shader.o figure.o mqoreader.o gear.o mqo_viewer_lib.js
	emcc -std=c++11 --js-library mqo_viewer_lib.js $(filter %.o,$^) -o mqo_viewer.js --embed-file data

.cpp.o:
	emcc -std=c++11 $< -o $@

clean:
	rm *.o mqo_viewer.js


