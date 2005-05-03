/*
 *  tile.h
 *  qthack
 *
 *  Created by Trenton Schulz on 3/27/05.
 *  Copyright 2005 Trolltech AS. All rights reserved.
 *
 */

#ifndef TILE_H
#define TILE_H

#include <qglobal.h>
#include <item.h>

class Tile
{
public:
    enum Type {
        Earth,
        Wall,
        ClosedDoor,
        OpenDoor,
        Floor
    };


    enum TileFlag {
        Default,
        Explored,
        Lit
    };
    Q_DECLARE_FLAGS(TileFlags, TileFlag)

    Tile (Type t = Earth, TileFlags f = Default) : type(t), flags(f) {}
    Type type;
    TileFlags flags;
    QList<const Item *> items;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileFlags);

#endif
