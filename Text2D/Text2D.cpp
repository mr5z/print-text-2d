#include <GL/glew.h>
#include <cstdlib>
#include <string>
#include "Text2D.h"
#include "BMfont.h"

const uint32_t QD = 4 * 2;

enum : char
{
     I_CR   = '\r'
    ,I_NL   = '\n'
    ,I_TAB  = '\t'
};

//4->QUAD
//2->DIMENSION

//reserve a size of QD * 20 = 160

std::vector< int >& getPosition() {
    static std::vector< int > t_vertices (QD * 20);
    return t_vertices;
}

std::vector< float >& getTexCoord() {
    static std::vector< float > t_coords (QD * 20);
    return t_coords;
}

std::vector< uint8_t  >& getColor() {
    static std::vector< uint8_t > t_colors (QD * 20);
    return t_colors;
}

std::vector< uint32_t >& getIndex() {
    static std::vector< uint32_t > t_indices (QD * 20);
    return t_indices;
}

void prepare(uint32_t length);
void compile(const BMfont&, const std::string&);
void shade(const Text2D::PenList &pen_list, uint32_t length);
void shade_text(const Text2D::Pen &pen);
void drawText(uint32_t texture_id);

void Text2D::printText(const BMfont &font, const char *text, const PenList &pen_list)
{
    std::string _text(text);

    if (_text.empty()) return;

    prepare(_text.length());
    compile(font, _text);
	shade(pen_list, _text.length());

    glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    drawText(font.textureSheet);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);

}

uint32_t Text2D::getWidth(const BMfont& f, const char *text)
{
	std::string _text(text);

    uint32_t temp_width = 0;
    uint32_t final_width = 0;

	for each (auto i in _text)
    {
        if (i == I_NL)
        {
            if (temp_width > final_width)
            {
                final_width = temp_width;
            }
            temp_width = 0;
        } else
        {
            temp_width += f.getGlyph(i).x_advance - f.getGlyph(i).x_ofs;
        }
    }

    return (temp_width > final_width) ? temp_width : final_width;

}

uint32_t Text2D::getHeight(const BMfont& font, const char *text)
{
    uint32_t nl_count = 1;
    do
    {
        if (*text == I_NL) ++nl_count;
    } while (*(++text));

    return font.lineHeight * nl_count;
}

void drawText(uint32_t texture)
{
    glVertexPointer  (2, GL_INT,           0, &getPosition()[0]);
    glColorPointer   (4, GL_UNSIGNED_BYTE, 0, &getColor()[0]);
    glTexCoordPointer(2, GL_FLOAT,         0, &getTexCoord()[0]);

    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_QUADS, getIndex().size(), GL_UNSIGNED_INT, &getIndex()[0]);

}

void prepare(uint32_t length)
{
    getPosition()   .clear();
    getColor()      .clear();
    getTexCoord()   .clear();
    getIndex()      .clear();

    getPosition()   .reserve(length * 4 * 2);
    getColor()      .reserve(length * 4 * 4);
    getTexCoord()   .reserve(length * 4 * 2);
    getIndex()      .reserve(length * 4 * 1);
}

void shade(const Text2D::PenList &pen_list, uint32_t length)
{
    uint32_t i = 0;
    //for(auto &pen : pen_list)		// C++11
    for each(auto &pen in pen_list) // MSVC++
    {
        auto begin = pen.begin;
        auto end = (pen.end > length ? length : pen.end);
        for (; i < end; ++i)
        {
            if (i >= begin && i <= end)
                //shade text with the 'pen' provided
                shade_text(pen);
            else
                //shade text with the default 'pen'
                shade_text(Text2D::def_pen.back());
        }
    }

    //shade the left text (if any)
    while (i++ < length)
    {
        shade_text(Text2D::def_pen.back());
    }
}

void compile(const BMfont& font, const std::string& text)
{
    int pen_x(0);
    int pen_y(0);
    int32_t index(-1);

    auto letter = text.begin();

    while (letter != text.end())
	{
	    if (*(letter) == I_TAB) { pen_x += font.tabSize;                ++letter; continue; }
        if (*(letter) == I_NL)  { pen_y += font.lineHeight; pen_x =  0; ++letter; continue; }

        int oem = static_cast<int>(*(letter++));

	    const BMchar& glyph(font.getGlyph(oem));

        oem *= QD;

        //position
        getPosition().push_back(pen_x + glyph.x_ofs);
        getPosition().push_back(pen_y + glyph.height + glyph.y_ofs);
        getPosition().push_back(pen_x + glyph.width + glyph.x_ofs);
        getPosition().push_back(pen_y + glyph.height + glyph.y_ofs);
        getPosition().push_back(pen_x + glyph.width + glyph.x_ofs);
        getPosition().push_back(pen_y + glyph.y_ofs);
        getPosition().push_back(pen_x + glyph.x_ofs);
        getPosition().push_back(pen_y + glyph.y_ofs);

        //texture coordinates
        getTexCoord().push_back(font.getTexBuffer(oem + 0));
        getTexCoord().push_back(font.getTexBuffer(oem + 1));
        getTexCoord().push_back(font.getTexBuffer(oem + 2));
        getTexCoord().push_back(font.getTexBuffer(oem + 3));
        getTexCoord().push_back(font.getTexBuffer(oem + 4));
        getTexCoord().push_back(font.getTexBuffer(oem + 5));
        getTexCoord().push_back(font.getTexBuffer(oem + 6));
        getTexCoord().push_back(font.getTexBuffer(oem + 7));

        //indices
        getIndex().push_back(++index);
        getIndex().push_back(++index);
        getIndex().push_back(++index);
        getIndex().push_back(++index);

        pen_x += (glyph.x_advance - glyph.x_ofs);

	}

}

void shade_text(const Text2D::Pen &pen)
{
    for (int i = 0;i < 4; ++i)
    {
        getColor().push_back(pen.r);
        getColor().push_back(pen.g);
        getColor().push_back(pen.b);
        getColor().push_back(pen.a);
    }
}
