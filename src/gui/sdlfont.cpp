/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2011-2013  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/sdlfont.h"

#include "client.h"
#include "graphics.h"
#include "logger.h"
#include "main.h"
#include "utils/paths.h"

#include "resources/image.h"
#include "resources/imagehelper.h"
#include "resources/resourcemanager.h"

#include <guichan/exception.hpp>

#include "debug.h"

const unsigned int CACHE_SIZE = 256;
const unsigned int CACHE_SIZE_SMALL1 = 2;
const unsigned int CACHE_SIZE_SMALL2 = 50;
const unsigned int CACHE_SIZE_SMALL3 = 170;
const unsigned int CLEAN_TIME = 7;
const int OUTLINE_SIZE = 1;

char *strBuf;

const unsigned int CACHES_NUMBER = 256;

#ifdef UNITTESTS
int sdlTextChunkCnt = 0;
#endif

SDLTextChunk::SDLTextChunk(const std::string &text0, const gcn::Color &color0,
              const gcn::Color &color1) :
    img(nullptr),
    text(text0),
    color(color0),
    color2(color1),
    next(nullptr)
{
#ifdef UNITTESTS
    sdlTextChunkCnt ++;
#endif
}

SDLTextChunk::~SDLTextChunk()
{
    delete img;
    img = nullptr;
#ifdef UNITTESTS
    sdlTextChunkCnt --;
#endif
}

bool SDLTextChunk::operator==(const SDLTextChunk &chunk) const
{
    return (chunk.text == text && chunk.color == color
            && chunk.color2 == color2);
}

void SDLTextChunk::generate(TTF_Font *const font, const float alpha)
{
    BLOCK_START("SDLTextChunk::generate")
    SDL_Color sdlCol;
    sdlCol.b = static_cast<uint8_t>(color.b);
    sdlCol.r = static_cast<uint8_t>(color.r);
    sdlCol.g = static_cast<uint8_t>(color.g);
    sdlCol.unused = 0;

    getSafeUtf8String(text, strBuf);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(
        font, strBuf, sdlCol);

    if (!surface)
    {
        img = nullptr;
        BLOCK_END("SDLTextChunk::generate")
        return;
    }

    const int width = surface->w;
    const int height = surface->h;

    if (color.r != color2.r || color.g != color2.g
        || color.b != color2.b)
    {   // outlining
        SDL_Color sdlCol2;
        SDL_Surface *background = imageHelper->create32BitSurface(
            width, height);
        if (!background)
        {
            img = nullptr;
            SDL_FreeSurface(surface);
            BLOCK_END("SDLTextChunk::generate")
            return;
        }
        sdlCol2.b = static_cast<uint8_t>(color2.b);
        sdlCol2.r = static_cast<uint8_t>(color2.r);
        sdlCol2.g = static_cast<uint8_t>(color2.g);
        sdlCol2.unused = 0;
        SDL_Surface *const surface2 = TTF_RenderUTF8_Blended(
            font, strBuf, sdlCol2);
        if (!surface2)
        {
            img = nullptr;
            SDL_FreeSurface(surface);
            BLOCK_END("SDLTextChunk::generate")
            return;
        }
        SDL_Rect rect =
        {
            OUTLINE_SIZE,
            0,
            static_cast<Uint16>(surface->w),
            static_cast<Uint16>(surface->h)
        };
//                SDL_SetAlpha(surface2, 0, SDL_ALPHA_OPAQUE);
        MSDL_gfxBlitRGBA(surface2, nullptr, background, &rect);
        rect.x = -OUTLINE_SIZE;
        MSDL_gfxBlitRGBA(surface2, nullptr, background, &rect);
        rect.x = 0;
        rect.y = -OUTLINE_SIZE;
        MSDL_gfxBlitRGBA(surface2, nullptr, background, &rect);
        rect.y = OUTLINE_SIZE;
        MSDL_gfxBlitRGBA(surface2, nullptr, background, &rect);
        rect.x = 0;
        rect.y = 0;
//                SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);
        MSDL_gfxBlitRGBA(surface, nullptr, background, &rect);
        SDL_FreeSurface(surface);
        SDL_FreeSurface(surface2);
        surface = background;
    }
    img = imageHelper->createTextSurface(
        surface, width, height, alpha);
    SDL_FreeSurface(surface);

    BLOCK_END("SDLTextChunk::generate")
}


TextChunkList::TextChunkList() :
    start(nullptr),
    end(nullptr),
    size(0)
{
}

void TextChunkList::insertFirst(SDLTextChunk *const item)
{
    SDLTextChunk *const oldFirst = start;
    if (start)
        start->prev = item;
    item->prev = nullptr;
    if (oldFirst)
        item->next = oldFirst;
    else
        end = item;
    start = item;
    size ++;
}

void TextChunkList::moveToFirst(SDLTextChunk *item)
{
    if (item == start)
        return;

    SDLTextChunk *oldPrev = item->prev;
    if (oldPrev)
        oldPrev->next = item->next;
    SDLTextChunk *oldNext = item->next;
    if (oldNext)
        oldNext->prev = item->prev;
    else
        end = oldPrev;
    SDLTextChunk *const oldFirst = start;
    if (start)
        start->prev = item;
    item->prev = nullptr;
    item->next = oldFirst;
    start = item;
}

void TextChunkList::removeBack()
{
    SDLTextChunk *oldEnd = end;
    if (oldEnd)
    {
        end = oldEnd->prev;
        if (end)
            end->next = nullptr;
        else
            start = nullptr;
        delete oldEnd;
        size --;
    }
}

void TextChunkList::removeBack(int n)
{
    SDLTextChunk *item = end;
    while (n && item)
    {
        n --;
        SDLTextChunk *oldEnd = item;
        item = item->prev;
        delete oldEnd;
        size --;
    }
    if (item)
    {
        item->next = nullptr;
        end = item;
    }
    else
    {
        start = nullptr;
        end = nullptr;
    }
}

void TextChunkList::clear()
{
    SDLTextChunk *item = start;
    while (item)
    {
        SDLTextChunk *item2 = item->next;
        delete item;
        item = item2;
    }
    start = nullptr;
    end = nullptr;
    size = 0;
}

namespace
{
    TextChunkList mCache[CACHES_NUMBER];
}  // namespace

static int fontCounter;

SDLFont::SDLFont(std::string filename, const int size, const int style) :
    mFont(nullptr),
    mCreateCounter(0),
    mDeleteCounter(0),
    mCleanTime(cur_time + CLEAN_TIME)
{
    const ResourceManager *const resman = ResourceManager::getInstance();

    if (fontCounter == 0 && TTF_Init() == -1)
    {
        throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
            std::string(TTF_GetError()));
    }

    if (!fontCounter)
    {
        strBuf = new char[65535];
        memset(strBuf, 0, 65535);
    }

    ++fontCounter;

    fixDirSeparators(filename);
    mFont = TTF_OpenFont(resman->getPath(filename).c_str(), size);

    if (!mFont)
    {
        logger->log("Error finding font " + filename);
        std::string backFile = "fonts/dejavusans.ttf";
        mFont = TTF_OpenFont(resman->getPath(
            fixDirSeparators(backFile)).c_str(), size);
        if (!mFont)
        {
            throw GCN_EXCEPTION("SDLSDLFont::SDLSDLFont: " +
                                std::string(TTF_GetError()));
        }
    }

    TTF_SetFontStyle(mFont, style);
}

SDLFont::~SDLFont()
{
    TTF_CloseFont(mFont);
    mFont = nullptr;
    --fontCounter;
    clear();

    if (fontCounter == 0)
    {
        TTF_Quit();
        delete []strBuf;
    }
}

void SDLFont::loadFont(std::string filename, const int size, const int style)
{
    const ResourceManager *const resman = ResourceManager::getInstance();

    if (fontCounter == 0 && TTF_Init() == -1)
    {
        logger->log("Unable to initialize SDL_ttf: " +
                    std::string(TTF_GetError()));
        return;
    }

    fixDirSeparators(filename);
    TTF_Font *const font = TTF_OpenFont(
        resman->getPath(filename).c_str(), size);

    if (!font)
    {
        logger->log("SDLSDLFont::SDLSDLFont: " +
                    std::string(TTF_GetError()));
        return;
    }

    if (mFont)
        TTF_CloseFont(mFont);

    mFont = font;
    TTF_SetFontStyle(mFont, style);
    clear();
}

void SDLFont::clear()
{
    for (size_t f = 0; f < CACHES_NUMBER; f ++)
        mCache[f].clear();
}

void SDLFont::drawString(gcn::Graphics *const graphics,
                         const std::string &text,
                         const int x, const int y)
{
    BLOCK_START("SDLFont::drawString")
    if (text.empty())
    {
        BLOCK_END("SDLFont::drawString")
        return;
    }

    Graphics *const g = dynamic_cast<Graphics *const>(graphics);
    if (!g)
        return;

    gcn::Color col = g->getColor();
    gcn::Color col2 = g->getColor2();
    const float alpha = static_cast<float>(col.a) / 255.0f;

    /* The alpha value is ignored at string generation so avoid caching the
     * same text with different alpha values.
     */
    col.a = 255;

    SDLTextChunk chunk(text, col, col2);

    const unsigned char chr = text[0];
    TextChunkList *const cache = &mCache[chr];

    bool found = false;

#ifdef DEBUG_FONT
    int cnt = 0;
#endif

    SDLTextChunk *i = cache->start;
    while (i)
    {
        if (*i == chunk)
        {
            // Raise priority: move it to front
            cache->moveToFirst(i);
            found = true;
            break;
        }
        i = i->next;
#ifdef DEBUG_FONT
        cnt ++;
#endif
    }

#ifdef DEBUG_FONT
    logger->log(std::string("drawString: ").append(text).append(
        ", iterations: ").append(toString(cnt)));
#endif

    // Surface not found
    if (!found)
    {
        if (cache->size >= CACHE_SIZE)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter ++;
#endif
            cache->removeBack();
        }
#ifdef DEBUG_FONT_COUNTERS
        mCreateCounter ++;
#endif
        SDLTextChunk *chunk2 = new SDLTextChunk(text, col, col2);

        chunk2->generate(mFont, alpha);
        cache->insertFirst(chunk2);

        const Image *const image = chunk2->img;
        if (image)
            g->drawImage(image, x, y);
    }
    else
    {
        Image *const image = cache->start->img;
        if (image)
        {
            image->setAlpha(alpha);
            g->drawImage(image, x, y);
        }
    }
    BLOCK_END("SDLFont::drawString")
}

void SDLFont::slowLogic(const int rnd)
{
    BLOCK_START("SDLFont::slowLogic")
    if (!mCleanTime)
    {
        mCleanTime = cur_time + CLEAN_TIME + rnd;
    }
    else if (mCleanTime < cur_time)
    {
        doClean();
        mCleanTime = cur_time + CLEAN_TIME + rnd;
    }
    BLOCK_END("SDLFont::slowLogic")
}

int SDLFont::getWidth(const std::string &text) const
{
    if (text.empty())
        return 0;

    const unsigned char chr = text[0];
    TextChunkList *const cache = &mCache[chr];

#ifdef DEBUG_FONT
    int cnt = 0;
#endif

    SDLTextChunk *i = cache->start;
    while (i)
    {
        if (i->text == text)
        {
            // Raise priority: move it to front
            // Assumption is that TTF::draw will be called next
            cache->moveToFirst(i);
            const Image *const image = i->img;
            if (image)
                return image->getWidth();
            else
                return 0;
        }
        i = i->next;
#ifdef DEBUG_FONT
        cnt ++;
#endif
    }

#ifdef DEBUG_FONT
    logger->log(std::string("getWidth: ").append(text).append(
        ", iterations: ").append(toString(cnt)));
#endif

    int w, h;
    getSafeUtf8String(text, strBuf);
    TTF_SizeUTF8(mFont, strBuf, &w, &h);
    return w;
}

int SDLFont::getHeight() const
{
    return TTF_FontHeight(mFont);
}

void SDLFont::doClean()
{
    for (unsigned int f = 0; f < CACHES_NUMBER; f ++)
    {
        TextChunkList *const cache = &mCache[f];
        const size_t size = cache->size;
#ifdef DEBUG_FONT_COUNTERS
        logger->log("ptr: %d, size: %d", f, size);
#endif
        if (size > CACHE_SIZE_SMALL3)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter += 100;
#endif
            cache->removeBack(100);
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete3");
#endif
        }
        else if (size > CACHE_SIZE_SMALL2)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter += 20;
#endif
            cache->removeBack(20);
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete2");
#endif
        }
        else if (size > CACHE_SIZE_SMALL1)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter ++;
#endif
            cache->removeBack();
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete1");
#endif
        }
    }
}

TextChunkList *SDLFont::getCache()
{
    return mCache;
}
