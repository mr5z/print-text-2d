#pragma once

#include <vector>
#include <stdint.h>

class BMfont;

namespace Text2D
{

struct Pen
{
	Pen() :
		  r(0)
		, g(0)
		, b(0)
		, a(0)
	{

	}

	uint32_t begin;
    uint32_t end;

	uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

typedef std::vector<Pen> PenList;

static PenList def_pen(1, Pen());

void        printText   (const BMfont &font, const char *text, const PenList &pen_list = def_pen);
uint32_t    getWidth    (const BMfont &font, const char *text);
uint32_t    getHeight   (const BMfont &font, const char *text);

}
