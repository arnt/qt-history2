/****************************************************************************
**
** Definition of QMemoryManager class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMEMORYMANAGER_QWS_H
#define QMEMORYMANAGER_QWS_H

#ifndef QT_H
#include "qfontmanager_qws.h"
#include "qstring.h"
#include "qmap.h"
#include <private/qtextengine_p.h>
#endif // QT_H


class QFontDef;
class QMemoryManagerPixmap {
    friend class QMemoryManager;
    uchar* data;
    int xoffset;
};

class QMemoryManager {
public:
    QMemoryManager(
	void* vram, int vramsize,
	void* fontrom
	//, ...
    );

    // Pixmaps
    typedef int PixmapID;
    PixmapID newPixmap(int w, int h, int d, int optim );
    void deletePixmap(PixmapID);
    bool inVRAM(PixmapID) const;
    void findPixmap(PixmapID,
	    int width, int depth, // sames as passed when created
	    uchar** address, int* xoffset, int* linestep);

    // Fonts
    typedef void* FontID;
    FontID refFont(const QFontDef&);
    void derefFont(FontID);
    QRenderedFont* fontRenderer(FontID); // XXX JUST FOR METRICS
    bool inFont(FontID, glyph_t glyph) const;
    QGlyph lockGlyph(FontID, glyph_t glyph);
    QGlyphMetrics* lockGlyphMetrics(FontID, glyph_t glyph);
    void unlockGlyph(FontID, glyph_t glyph);
#ifndef QT_NO_QWS_SAVEFONTS
    void savePrerenderedFont(const QFontDef&, bool all=TRUE);
    void savePrerenderedFont(FontID id, bool all=TRUE);
#endif
    bool fontSmooth(FontID id) const;
    int fontAscent(FontID id) const;
    int fontDescent(FontID id) const;
    int fontMinLeftBearing(FontID id) const;
    int fontMinRightBearing(FontID id) const;
    int fontLeading(FontID id) const;
    int fontMaxWidth(FontID id) const;
    int fontUnderlinePos(FontID id) const;
    int fontLineWidth(FontID id) const;
    int fontLineSpacing(FontID id) const;

private:
    QMap<PixmapID,QMemoryManagerPixmap> pixmap_map;
    int next_pixmap_id;
    QMap<QString,FontID> font_map;
    int next_font_id;
};

extern QMemoryManager* memorymanager;

#endif // QMEMORYMANAGER_QWS_H
