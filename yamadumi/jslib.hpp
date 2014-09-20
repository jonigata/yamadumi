// 2014/09/20 Naoyuki Hirayama

#ifndef JSLIB_HPP_
#define JSLIB_HPP_

extern "C" {
void initMouse();
void initTextureVault();
void loadTexture(const char*);
void bindTexture(const char*);
}

#endif // JSLIB_HPP_
