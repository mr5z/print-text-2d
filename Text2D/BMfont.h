#pragma once

#include <stdint.h>

const uint32_t MAX_CHAR = 256;

enum BMfont_Status
{
    BMF_NO_ERROR,
    BMF_FNT_FILE_NOT_EXIST,
    BMF_TEX_FILE_NOT_EXIST,
    BMF_FNT_FILE_CORRUPTED,
    BMF_TEX_FILE_CORRUPTED,
    BMF_FNT_FILE_UNSUPPORTED
};

struct BMchar
{
    int x_ofs, y_ofs;
    uint32_t x, y;
    uint32_t width, height;
    uint32_t x_advance;
    uint32_t page;		// unuse
};

class BMfont
{
public:

	BMfont();
    ~BMfont();

	BMfont_Status load(const char* fontName);

	inline BMchar getGlyph(int index) const {
	    return glyphs[index];
    }

	inline float getTexBuffer(int index) const {
	    return texCoordBuff[index];
    }

    uint32_t edge_padding;
    uint32_t tabSize;
	uint32_t backTexture;
	uint32_t frontTexture;
	uint32_t textureSheet;
	bool enableMasking;

	uint32_t base;
	uint32_t lineHeight;
	uint32_t pages;
	uint32_t scaleW, scaleH;

private:

	BMchar  glyphs[MAX_CHAR];
	float   texCoordBuff[MAX_CHAR * 8];

    BMfont_Status parse(const char* fontFile);

};
