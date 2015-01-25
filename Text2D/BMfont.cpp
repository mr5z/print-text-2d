#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include <string.h>

#include <string>
#include <fstream>
#include <cstdlib>

#include "BMfont.h"

#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "SOIL")

//4->QUAD
//2->DIMENSION

const uint32_t QD = 4 * 2;

char texture_path[255];

const std::string getValue(const std::string& needle, const std::string& haystack, char delimeter);
bool keyExist(const std::string& keyToFind, std::ifstream& file, std::string& line);
inline float invertY(float box_y) { return 1 - box_y; }

BMfont_Status BMfont::parse(const char* fontFile)
{
	std::ifstream file(fontFile, std::ios::in | std::ios::binary);

    if (file.bad())
        return BMF_FNT_FILE_NOT_EXIST;

    BMchar glyph;
    std::string line;
    std::string value;
    BMfont_Status status(BMF_NO_ERROR);

    uint32_t index;
    uint32_t maxCharContents;

    do
    {
        if (keyExist("common", file, line))
        {

            value = getValue("lineHeight", line, '='); lineHeight = std::atoi(value.c_str());
            value = getValue("base"      , line, '='); base       = std::atoi(value.c_str());
            value = getValue("scaleW"    , line, '='); scaleW     = std::atoi(value.c_str());
            value = getValue("scaleH"    , line, '='); scaleH     = std::atoi(value.c_str());
            value = getValue("pages"     , line, '='); pages      = std::atoi(value.c_str());

        } else
        {
            status = BMF_FNT_FILE_CORRUPTED;
            break;
        }

        if (keyExist("page", file, line))
            strcat_s(texture_path, getValue("file", line, '=').c_str());
        else
        {
            status = BMF_FNT_FILE_CORRUPTED;
            break;
        }

        if (keyExist("chars", file, line))
            value = getValue("count", line, '=');
        else
        {
            status = BMF_FNT_FILE_CORRUPTED;
            break;
        }

        maxCharContents = std::atoi(value.c_str());

        if (maxCharContents <= 0 || maxCharContents >= MAX_CHAR)
        {
            status = BMF_FNT_FILE_UNSUPPORTED; //we only support OEM characters
            break;
        }

        //we currently don't support kernings at this time
        for (uint32_t i = 0;i < maxCharContents; ++i)
        {
            getline(file, line);

            value = getValue("id", line, '='); index = std::atoi(value.c_str());

            if (index < MAX_CHAR)
            {
                value = getValue("x"       , line, '='); glyph.x         = std::atoi(value.c_str());
                value = getValue("y"       , line, '='); glyph.y         = std::atoi(value.c_str());
                value = getValue("width"   , line, '='); glyph.width     = std::atoi(value.c_str());
                value = getValue("height"  , line, '='); glyph.height    = std::atoi(value.c_str());
                value = getValue("xoffset" , line, '='); glyph.x_ofs     = std::atoi(value.c_str());
                value = getValue("yoffset" , line, '='); glyph.y_ofs     = std::atoi(value.c_str());
                value = getValue("xadvance", line, '='); glyph.x_advance = std::atoi(value.c_str());
                value = getValue("page"    , line, '='); glyph.page      = std::atoi(value.c_str());

                glyphs[index] = glyph;
                index *= QD;

                texCoordBuff[index + 0] = (float)glyph.x / (float)scaleW;
                texCoordBuff[index + 1] = invertY((float)(glyph.y + glyph.height) / (float)scaleH);
                texCoordBuff[index + 2] = (float)(glyph.x + glyph.width) / (float)scaleW;
                texCoordBuff[index + 3] = invertY((float)(glyph.y + glyph.height) / (float)scaleH);
                texCoordBuff[index + 4] = (float)(glyph.x + glyph.width) / (float)scaleW;
                texCoordBuff[index + 5] = invertY((float)glyph.y / (float)scaleH);
                texCoordBuff[index + 6] = (float)glyph.x / (float)scaleW;
                texCoordBuff[index + 7] = invertY((float)glyph.y / (float)scaleH);
            }

        }
    } while (false);

    file.close();
    return status;

}

BMfont_Status BMfont::load(const char* fontFile)
{
    memset(texture_path,0,sizeof(texture_path));

    auto str_len = strlen(fontFile);

    for (int i = str_len - 1;i >= 0 ; --i)
    {
        // support for x platforms directory separator
        if (fontFile[i] == '\\' || fontFile[i] == '/')
        {
            for (int j = 0;j <= i; ++j)
            {
                texture_path[j] = fontFile[j];
            }
            break;
        }
    }

    BMfont_Status fnt_status = parse(fontFile);
    if (fnt_status != BMF_NO_ERROR)
        return fnt_status;

    textureSheet = SOIL_load_OGL_texture
	(
    texture_path,
    SOIL_LOAD_AUTO,
    SOIL_CREATE_NEW_ID,
    SOIL_FLAG_INVERT_Y// | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS
	);

	if ( textureSheet == 0 )
    {
		glDeleteTextures(1, &textureSheet);
		printf("soil: %s", SOIL_last_result());
		return BMF_TEX_FILE_NOT_EXIST;
	}

	return BMF_NO_ERROR;
}

BMfont::BMfont() :
     edge_padding    (4)
    ,tabSize         (40)
    ,enableMasking   (false)
	,lineHeight      (1)
{

}

BMfont::~BMfont()
{
	//glDeleteTextures(1, &textureSheet);
}

const std::string getValue(const std::string& needle, const std::string& haystack, char delimeter)
{
    std::string value;

    for (uint32_t i = 0; i < haystack.length(); ++i)
    {
        if (haystack[i] == needle[0])
        {

            value = "";

            uint32_t duration = i + needle.length();

            if (duration > haystack.length()) return "";

            for (uint32_t j = i;j < duration; ++j)
            {

                value += haystack[j];
                if (value == needle)
                {

                    value = "";

                    while (haystack[j++] != delimeter)
                    {
                        if (j > haystack.length()) return "";
                    }

                    if (haystack[j] == '"') ++j;

                    while (true)
                    {
                        const char& CHAR = haystack[j];
                        if (CHAR == ' '  ||
                            CHAR == '\r' || // Windows "\r\n" newline format
                            CHAR == '\n' ||
                            CHAR == '\t' ||
                            CHAR == '"'  ||
                            CHAR == '\0' )
                            break;
                        value += CHAR;
                        ++j;
                    }

                    return value;
                }
            }
        }

    }
    return "";
}

bool keyExist(const std::string& keyToFind, std::ifstream& file, std::string& line)
{
    while (file.good())
    {
        getline(file, line);
        for (uint32_t i = 0;i < line.length(); ++i)
        {
            if (line[i] == keyToFind[0])
            {
                std::string value;
                for (unsigned int j = i;j < i + keyToFind.length(); ++j)
                {
                    value += line[j];
                    if (value == keyToFind) return true;
                    if (j >= line.length()) break;
                }
            }
        }
    }

    return false;
}

