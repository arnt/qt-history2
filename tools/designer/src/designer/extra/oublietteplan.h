/*
 *  oublietteplan.h
 *  qthack
 *
 *  Created by Trenton Schulz on 3/24/05.
 *  Copyright 2005 Trolltech AS. All rights reserved.
 *
 */

#ifndef OUBLIETTELEVEL_H
#define OUBLIETTELEVEL_H

#include <QtCore/QtCore>
#include "tile.h"

class OublietteLevel
{
public:
    enum LevelFeature {
        HorizontalCorridor,
        SmallHorizontalCorridor,
        VerticalCorridor,
        SmallVerticalCorridor,
        ThreeByThreeRoom,
        FiveByFiveRoom,
        TenByTenRoom,
        TotalFeatures
    };

    OublietteLevel(int width = 128, int height = 96, int totalFeatures = 200);
    ~OublietteLevel();
    inline QSize size() const { return m_size; }
    inline int width() const { return size().width(); }
    inline int height() const { return size().height(); }
    inline int totalItems() const { return m_totalItems; }
    Tile tile(int x, int y) const;
    inline Tile tile(const QPoint &pos) const { return tile(pos.x(), pos.y()); }
    void updateTileFlags(int x, int y, Tile::TileFlags flags);
    inline void updateTileFlags(const QPoint &point, Tile::TileFlags flags)
    { updateTileFlags(point.x(), point.y(), flags); }

    void addItemToTile(int x, int y, const Item *item);
    inline void addItemToTile(const QPoint &pos, const Item *item)
    { addItemToTile(pos.x(), pos.y(), item); }

    void clearTileFlags(int x, int y);
    inline void clearTileFlags(const QPoint &point) { clearTileFlags(point.x(), point.y()); }
    void printOubliette() const;
    bool blockLOS(int x, int y);
    void setTile(int x, int y, Tile element);
    inline void setTile(const QPoint &pos, Tile element)
    { setTile(pos.x(), pos.y(), element); }

private:
    void generateOubliette();
    void dig(LevelFeature feature, const QRect &rect);
    void digRoom(const QRect &rect, Tile::TileFlags flags = Tile::Default);
    QPoint findWall() const;
    LevelFeature pickFeature() const;
    QRect boundingRect(LevelFeature feature) const;
    bool placeFeature(LevelFeature feature, const QPoint &pt);
    inline void fillRect(const QRect &r, Tile le)
    { fillRect(r.x(), r.y(), r.width(), r.height(), le); }
    void fillRect(int x, int y, int width, int height, Tile le);
    bool checkForFeatures(const QRect &rect) const;

private:
    QSize m_size;
    Tile *m_map;
    int m_totalFeatures;
    int m_totalItems;
    QList<QRect> m_roomList;
};

class OubliettePlan
{
public:
    OubliettePlan(int totalLevels = 1);
    ~OubliettePlan();
    OublietteLevel *level(int level) const;
    inline int totalLevels() const { return m_totalLevels; }

private:
    QVector<OublietteLevel*> m_levels;
    int m_totalLevels;
};

#endif // OUBLIETTELEVEL_H
