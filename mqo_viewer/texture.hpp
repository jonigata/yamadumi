// 2014/09/18 Naoyuki Hirayama

/*!
	@file	  texture.hpp
	@brief	  <�T�v>

	<����>
*/

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

extern "C" {
void initTextureVault();
void loadTexture(const char*);
void bindTexture(const char*);
}

#endif // TEXTURE_HPP_
