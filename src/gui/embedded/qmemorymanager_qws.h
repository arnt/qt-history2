/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMEMORYMANAGER_QWS_H
#define QMEMORYMANAGER_QWS_H

#include "QtCore/qstring.h"
#include "QtCore/qmap.h"
#include <QtGui/private/qtextengine_p.h>

class QMemoryManagerPixmap {
    friend class QMemoryManager;
    uchar* data;
    int xoffset;
};

class QMemoryManager {
public:
    QMemoryManager(void* vram, int vramsize, void* fontrom);

    // Pixmaps
    typedef int PixmapID;
    PixmapID newPixmap(int w, int h, int d);
    void deletePixmap(PixmapID);
    bool inVRAM(PixmapID) const;
    void findPixmap(PixmapID,
            int width, int depth, // sames as passed when created
            uchar** address, int* xoffset, int* linestep);


private:
    QMap<PixmapID,QMemoryManagerPixmap> pixmap_map;
    int next_pixmap_id;
};

extern QMemoryManager* memorymanager;

#endif // QMEMORYMANAGER_QWS_H
