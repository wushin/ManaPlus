/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2015  The ManaPlus Developers
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

#include "resources/map/speciallayer.h"

#include "resources/mapitemtype.h"

#include "resources/map/mapconsts.h"
#include "resources/map/mapitem.h"

#include "utils/delete2.h"

#include "debug.h"

SpecialLayer::SpecialLayer(const int width, const int height) :
    mWidth(width),
    mHeight(height),
    mTiles(new MapItem*[mWidth * mHeight])
{
    std::fill_n(mTiles, mWidth * mHeight, static_cast<MapItem*>(nullptr));
}

SpecialLayer::~SpecialLayer()
{
    for (int f = 0; f < mWidth * mHeight; f ++)
        delete2(mTiles[f])
    delete [] mTiles;
}

MapItem* SpecialLayer::getTile(const int x, const int y) const
{
    if (x < 0 || x >= mWidth ||
        y < 0 || y >= mHeight)
    {
        return nullptr;
    }
    return mTiles[x + y * mWidth];
}

void SpecialLayer::setTile(const int x, const int y, MapItem *const item)
{
    if (x < 0 || x >= mWidth ||
        y < 0 || y >= mHeight)
    {
        return;
    }

    const int idx = x + y * mWidth;
    delete mTiles[idx];
    if (item)
        item->setPos(x, y);
    mTiles[idx] = item;
}

void SpecialLayer::setTile(const int x, const int y, const int type)
{
    if (x < 0 || x >= mWidth ||
        y < 0 || y >= mHeight)
    {
        return;
    }

    const int idx = x + y * mWidth;
    MapItem *const tile = mTiles[idx];
    if (tile)
    {
        tile->setType(type);
        tile->setPos(x, y);
    }
    else
    {
        mTiles[idx] = new MapItem(type);
        mTiles[idx]->setPos(x, y);
    }
}

void SpecialLayer::addRoad(const Path &road)
{
    FOR_EACH (Path::const_iterator, i, road)
    {
        const Position &pos = (*i);
        MapItem *const item = getTile(pos.x, pos.y);
        if (!item)
            setTile(pos.x, pos.y, new MapItem(MapItemType::ROAD));
        else
            item->setType(MapItemType::ROAD);
    }
}

void SpecialLayer::clean() const
{
    if (!mTiles)
        return;

    for (int f = 0; f < mWidth * mHeight; f ++)
    {
        MapItem *const item = mTiles[f];
        if (item)
            item->setType(MapItemType::EMPTY);
    }
}

void SpecialLayer::draw(Graphics *const graphics, int startX, int startY,
                        int endX, int endY,
                        const int scrollX, const int scrollY) const
{
    BLOCK_START("SpecialLayer::draw")
    if (startX < 0)
        startX = 0;
    if (startY < 0)
        startY = 0;
    if (endX > mWidth)
        endX = mWidth;
    if (endY > mHeight)
        endY = mHeight;

    for (int y = startY; y < endY; y ++)
    {
        const int py = y * mapTileSize - scrollY;
        const int y2 = y * mWidth;
        for (int x = startX; x < endX; x ++)
        {
            const MapItem *const item = mTiles[x + y2];
            if (item)
            {
                item->draw(graphics, x * mapTileSize - scrollX, py,
                    mapTileSize, mapTileSize);
            }
        }
    }
    BLOCK_END("SpecialLayer::draw")
}
