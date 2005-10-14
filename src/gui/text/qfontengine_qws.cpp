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

#include "qfontengine_p.h"
#include <private/qunicodetables_p.h>
#include <qwsdisplay_qws.h>
#include <qvarlengtharray.h>
#include <private/qpainter_p.h>
#include <private/qpaintengine_raster_p.h>
#include "qtextengine_p.h"
#include "qopentype_p.h"

#include <qdebug.h>


#ifndef QT_NO_QWS_QPF

#include "qfile.h"
#include "qdir.h"

#define QT_USE_MMAP
#include <stdlib.h>

#ifdef QT_USE_MMAP
// for mmap
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#endif

#endif // QT_NO_QWS_QPF




#ifndef QT_NO_FREETYPE

#include FT_TRUETYPE_TABLES_H

FT_Library QFontEngineFT::ft_library = 0;

class QGlyph {
public:
    QGlyph() :
        advance(0),
        data(0) {}
    ~QGlyph() {}

    quint8 pitch;
    quint8 width;
    quint8 height;

    qint8 bearingx;      // Difference from pen position to glyph's left bbox
    quint8 advance;       // Difference between pen positions
    qint8 bearingy;      // Used for putting characters on baseline

    bool mono :1;
    uint reserved:15;
    uchar* data;
};

static void render(FT_Face face, glyph_t index, QGlyph *result, bool smooth)
{
    FT_Error err;

    err=FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
    if (err) {
        qDebug("failed loading glyph %d from font", index);
        Q_ASSERT(!err);
    }

    if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        FT_Render_Mode render_mode = FT_RENDER_MODE_NORMAL;
        if (!smooth)
            render_mode = FT_RENDER_MODE_MONO;
        err = FT_Render_Glyph(face->glyph, render_mode);
        if (err && err != -1 && face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0) {
            qDebug("failed rendering glyph %d from font", index);
            Q_ASSERT(!err );
        }
    }

    FT_Bitmap bm = face->glyph->bitmap;
    result->pitch = bm.pitch;
    result->width = bm.width;
    result->height = bm.rows;
    if (result->pitch == 0 || result->height == 0 || result->width == 0) {
        result->pitch = 0;
        result->data = 0;
        return;
    }

    int size = bm.pitch*bm.rows;
    result->data = new uchar[size];
    result->mono = bm.pixel_mode == ft_pixel_mode_mono;
    if (size) {
        memcpy(result->data, bm.buffer, size);
    } else {
        result->data = 0;
    }
    result->bearingx = face->glyph->metrics.horiBearingX/64;
    result->advance = face->glyph->metrics.horiAdvance/64;
    result->bearingy = face->glyph->metrics.horiBearingY/64;
}


FT_Face QFontEngineFT::handle() const
{
    return face;
}

QFontEngineFT::QFontEngineFT(const QFontDef& d, FT_Face ft_face)
{
    _openType = 0;
    fontDef = d;
    face = ft_face;

    smooth = FT_IS_SCALABLE(face);
    if (fontDef.styleStrategy & QFont::NoAntialias)
        smooth = false;
    rendered_glyphs = new QGlyph *[face->num_glyphs];
    memset(rendered_glyphs, 0, face->num_glyphs*sizeof(QGlyph *));
    cache_cost = face->num_glyphs*6*8; // ##########
    memset(cmapCache, 0, sizeof(cmapCache));
}

QFontEngineFT::~QFontEngineFT()
{
    for (int i = 0; i < face->num_glyphs; ++i)
        delete rendered_glyphs[i];
    delete [] rendered_glyphs;
    FT_Done_Face(face);
}


QFontEngine::FECaps QFontEngineFT::capabilites() const
{
    return NoTransformations;
}

inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

/* returns 0 as glyph index for non existant glyphs */
bool QFontEngineFT::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    int glyph_pos = 0;
    if (flags & QTextEngine::RightToLeft) {
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = QUnicodeTables::mirroredChar(getChar(str, i, len));
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
                if (uc < cmapCacheSize)
                    cmapCache[uc] = glyphs[glyph_pos].glyph;
            }
            ++glyph_pos;
        }
    } else {
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = getChar(str, i, len);
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
                if (uc < cmapCacheSize)
                    cmapCache[uc] = glyphs[glyph_pos].glyph;
            }
            ++glyph_pos;
        }
    }
    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);
    glyph_pos = 0;
    return true;
}


void QFontEngineFT::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &si)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);
    //##### Q_ASSERT(p->d_func()->txop < QPainterPrivate::TxScale);
    if (1) { //####### p->d_func()->txop == QPainterPrivate::TxTranslate) {
        QPointF tmpPt(_x, _y);
        tmpPt = tmpPt * pState->matrix();
        _x = tmpPt.x();
        _y = tmpPt.y();
    }

    QFixed x = QFixed::fromReal(_x);
    QFixed y = QFixed::fromReal(_y);

    if (si.flags) {
        int lw = qRound(lineThickness());
        lw = qMax(1, lw);
        if(si.width != 0 && si.flags != 0) {
            if(si.flags & QTextItem::Underline)
                paintEngine->qwsFillRect(qRound(x), qRound(y + underlinePosition()), qRound(si.width), lw);
            if(si.flags & QTextItem::Overline)
                paintEngine->qwsFillRect(qRound(x), qRound(y - (ascent() + 1)), qRound(si.width), lw);
            if(si.flags & QTextItem::StrikeOut)
                paintEngine->qwsFillRect(qRound(x), qRound(y - (ascent() / 3)), qRound(si.width), lw);
        }
    }

    QGlyphLayout *glyphs = si.glyphs;

    if (si.flags & QTextItem::RightToLeft) {
        for(int i = si.num_glyphs - 1; i >= 0; --i) {
            const QGlyphLayout *g = glyphs + i;
            const QGlyph *glyph = rendered_glyphs[g->glyph];
            Q_ASSERT(glyph);

            if(glyph->data)
                paintEngine->alphaPenBlt(glyph->data, glyph->pitch, glyph->mono,
                                         qRound(x + g->offset.x) + glyph->bearingx,
                                         qRound(y + g->offset.y) - glyph->bearingy,
                                         glyph->width,glyph->height);
            x += g->advance.x;
        }
    } else {
        for(int i = 0; i < si.num_glyphs; i++) {
            const QGlyphLayout *g = glyphs + i;
            const QGlyph *glyph = rendered_glyphs[g->glyph];
            Q_ASSERT(glyph);

            if(glyph->data)
                paintEngine->alphaPenBlt(glyph->data, glyph->pitch, glyph->mono,
                                         qRound(x + g->offset.x) + glyph->bearingx,
                                         qRound(y + g->offset.y) - glyph->bearingy,
                                         glyph->width,glyph->height);
            x += g->advance.x;
        }
    }
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    const QGlyph *g = rendered_glyphs[glyph];
    Q_ASSERT(g);
    return glyph_metrics_t(g->bearingx, -g->bearingy,
                            g->width, g->height,
                            g->advance, 0);
}

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (FT_IS_SCALABLE(face)) {
        QPointF point = QPointF(x, y);
        if (flags & QTextItem::RightToLeft) {
            for (int gl = 0; gl < numGlyphs; gl++)
                point += glyphs[gl].advance.toPointF();
        }
        for (int gl = 0; gl < numGlyphs; gl++) {
            FT_UInt glyph = glyphs[gl].glyph;
            if (flags & QTextItem::RightToLeft)
                point -= glyphs[gl].advance.toPointF();
            QPointF cp = point + glyphs[gl].offset.toPointF();
            if (!(flags & QTextItem::RightToLeft))
                point += glyphs[gl].advance.toPointF();

            FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);

            FT_GlyphSlot g = face->glyph;
            if (g->format != FT_GLYPH_FORMAT_OUTLINE)
                continue;

            // convert the outline to a painter path
            int i = 0;
            for (int c = 0; c < g->outline.n_contours; ++c) {
                int last_point = g->outline.contours[c];
                QPointF start = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
                if(!(g->outline.tags[i] & 1)) {
                    start += cp + QPointF(g->outline.points[last_point].x/64., -g->outline.points[last_point].y/64.);
                    start /= 2;
                }
//                 qDebug("contour: %d -- %d", i, g->outline.contours[c]);
//                 qDebug("first point at %f %f", start.x(), start.y());
                path->moveTo(start);

                QPointF c[4];
                c[0] = start;
                int n = 1;
                while (i < last_point) {
                    ++i;
                    c[n] = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
//                     qDebug() << "    i=" << i << " flag=" << (int)g->outline.tags[i] << "point=" << c[n];
                    ++n;
                    switch (g->outline.tags[i] & 3) {
                    case 2:
                        // cubic bezier element
                        if (n < 4)
                            continue;
                        c[3] = (c[3] + c[2])/2;
                        --i;
                        break;
                    case 0:
                        // quadratic bezier element
                        if (n < 3)
                            continue;
                        c[3] = (c[1] + c[2])/2;
                        c[2] = (2*c[1] + c[3])/3;
                        c[1] = (2*c[1] + c[0])/3;
                        --i;
                        break;
                    case 1:
                    case 3:
                        if (n == 2) {
//                             qDebug() << "lineTo" << c[1];
                            path->lineTo(c[1]);
                            c[0] = c[1];
                            n = 1;
                            continue;
                        } else if (n == 3) {
                            c[3] = c[2];
                            c[2] = (2*c[1] + c[3])/3;
                            c[1] = (2*c[1] + c[0])/3;
                        } 
                        break;
                    }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
                    path->cubicTo(c[1], c[2], c[3]);
                    c[0] = c[3];
                    n = 1;
                }
                if (n == 1) {
//                     qDebug() << "closeSubpath";
                    path->closeSubpath();
                } else {
                    c[3] = start;
                    if (n == 2) {
                        c[2] = (2*c[1] + c[3])/3;
                        c[1] = (2*c[1] + c[0])/3;
                    }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
                    path->cubicTo(c[1], c[2], c[3]);
                }
                ++i;
            }
        }
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
    }
}

bool QFontEngineFT::canRender(const QChar *string,  int len)
{
    int glyph_pos = 0;
    for ( int i = 0; i < len; ++i ) {
        unsigned int uc = getChar(string, i, len);
        if (uc == 0xa0) uc = 0x20;
        if (!FT_Get_Char_Index(face, uc))
            return false;
        ++glyph_pos;
    }

    return true;
}

#define FLOOR(x)  ((x) & -64)
#define CEIL(x)   (((x)+63) & -64)
#define TRUNC(x)  ((x) >> 6)

QFixed QFontEngineFT::ascent() const
{
    return QFixed::fromFixed(face->size->metrics.ascender);
}

QFixed QFontEngineFT::descent() const
{
    return QFixed::fromFixed(-face->size->metrics.descender);
}

QFixed QFontEngineFT::leading() const
{
    return QFixed::fromFixed(face->size->metrics.height
            - face->size->metrics.ascender /*ascent*/
            + face->size->metrics.descender);
}

qreal QFontEngineFT::maxCharWidth() const
{
    return face->size->metrics.max_advance/64.;
}

QFixed QFontEngineFT::xHeight() const
{
    TT_PCLT *pct = (TT_PCLT *)FT_Get_Sfnt_Table(face, ft_sfnt_pclt);
    if (pct && pct->xHeight) {
        return QFixed(pct->xHeight*face->size->metrics.y_ppem)/face->units_per_EM;
    }
    return QFontEngine::xHeight();
}

qreal QFontEngineFT::minLeftBearing() const
{
    return 0;
//     return (memorymanager->fontMinLeftBearing(handle()))>>8;
}

qreal QFontEngineFT::minRightBearing() const
{
    return 0;
//     return (memorymanager->fontMinRightBearing(handle()))>>8;
}

QFixed QFontEngineFT::underlinePosition() const
{
    if (FT_IS_SCALABLE(face))
        return QFixed::fromFixed(-FT_MulFix(face->underline_position, face->size->metrics.y_scale));
    else
         return ((lineThickness() * 2) + 3) / 6;
}

QFixed QFontEngineFT::lineThickness() const
{
    if (FT_IS_SCALABLE(face))
        return QFixed::fromFixed(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale));

// copied from QFontEngineQPF

    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

QFontEngine::Type QFontEngineFT::type() const
{
    return Freetype;
}


QOpenType *QFontEngineFT::openType() const
{
//     qDebug("openTypeIface requested!");
    if (_openType)
        return _openType;

    if (!FT_IS_SFNT(face))
        return 0;

    QFontEngineFT *that = const_cast<QFontEngineFT *>(this);
    that->_openType = new QOpenType(that, that->face);
    return _openType;
}

void QFontEngineFT::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::DesignMetrics) {
        for (int i = 0; i < len; i++) {
            FT_Load_Glyph(face, glyphs[i].glyph, FT_LOAD_NO_HINTING);
            glyphs[i].advance.x = QFixed::fromFixed(face->glyph->metrics.horiAdvance);
            glyphs[i].advance.y = 0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            FT_UInt g = glyphs[i].glyph;
            if (!rendered_glyphs[g]) {
                rendered_glyphs[g] = new QGlyph;
                render(face, g, rendered_glyphs[g], smooth);
            }
            glyphs[i].advance.x = rendered_glyphs[g]->advance;
            glyphs[i].advance.y = 0;
        }
    }
}

void QFontEngineFT::doKerning(int num_glyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (FT_HAS_KERNING(face)) {
        uint f = (flags == QTextEngine::DesignMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT);
        for (int i = 0; i < num_glyphs-1; ++i) {
            FT_Vector kerning;
            FT_Get_Kerning(face, glyphs[i].glyph, glyphs[i+1].glyph, f, &kerning);
            glyphs[i].advance.x += QFixed::fromFixed(kerning.x);
            glyphs[i].advance.y += QFixed::fromFixed(kerning.y);
        }
    }
}

#endif // QT_NO_FREETYPE


QFixed QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

QFixed QFontEngine::underlinePosition() const
{
    return ((lineThickness() * 2) + 3) / 6;
}


QFontEngine::~QFontEngine()
{
}





#ifndef QT_NO_QWS_QPF


#define FM_SMOOTH 1


class Q_PACKED QPFGlyphMetrics {

public:
    quint8 linestep;
    quint8 width;
    quint8 height;
    quint8 flags;

    qint8 bearingx;      // Difference from pen position to glyph's left bbox
    quint8 advance;       // Difference between pen positions
    qint8 bearingy;      // Used for putting characters on baseline

    qint8 reserved;      // Do not use

    // Flags:
    // RendererOwnsData - the renderer is responsible for glyph data
    //                    memory deletion otherwise QPFGlyphTree must
    //                    delete [] the data when the glyph is deleted.
    enum Flags { RendererOwnsData=0x01 };
};

class QPFGlyph {
public:
    QPFGlyph() { metrics=0; data=0; }
    QPFGlyph(QPFGlyphMetrics* m, uchar* d) :
	metrics(m), data(d) { }
    ~QPFGlyph() {}

    QPFGlyphMetrics* metrics;
    uchar* data;
};

struct Q_PACKED QPFFontMetrics{
    qint8 ascent,descent;
    qint8 leftbearing,rightbearing;
    quint8 maxwidth;
    qint8 leading;
    quint8 flags;
    quint8 underlinepos;
    quint8 underlinewidth;
    quint8 reserved3;
};


class QPFGlyphTree {
public:
    /* reads in a tree like this:

       A-Z
       /   \
       0-9   a-z

       etc.

    */
    glyph_t min,max;
    QPFGlyphTree* less;
    QPFGlyphTree* more;
    QPFGlyph* glyph;
public:
#ifdef QT_USE_MMAP
    QPFGlyphTree(uchar*& data)
    {
        read(data);
    }
#else
    QPFGlyphTree(QIODevice& f)
    {
        read(f);
    }
#endif

    ~QPFGlyphTree()
    {
        // NOTE: does not delete glyph[*].metrics or .data.
        //       the caller does this (only they know who owns
        //       the data).  See clear().
        delete less;
        delete more;
        delete [] glyph;
    }

    bool inFont(glyph_t g) const
    {
        if ( g < min ) {
            if ( !less )
                return false;
            return less->inFont(g);
        } else if ( g > max ) {
            if ( !more )
                return false;
            return more->inFont(g);
        }
        return true;
    }

    QPFGlyph* get(glyph_t g)
    {
        if ( g < min ) {
            if ( !less )
                return 0;
            return less->get(g);
        } else if ( g > max ) {
            if ( !more )
                return 0;
            return more->get(g);
        }
        return &glyph[g - min];
    }
    int totalChars() const
    {
        if ( !this ) return 0;
        return max-min+1 + less->totalChars() + more->totalChars();
    }
    int weight() const
    {
        if ( !this ) return 0;
        return 1 + less->weight() + more->weight();
    }

    void dump(int indent=0)
    {
        for (int i=0; i<indent; i++) printf(" ");
        printf("%d..%d",min,max);
        //if ( indent == 0 )
        printf(" (total %d)",totalChars());
        printf("\n");
        if ( less ) less->dump(indent+1);
        if ( more ) more->dump(indent+1);
    }

private:
    QPFGlyphTree()
    {
    }

#ifdef QT_USE_MMAP
    void read(uchar*& data)
    {
        // All node data first
        readNode(data);
        // Then all non-video data
        readMetrics(data);
        // Then all video data
        readData(data);
    }
#else
    void read(QIODevice& f)
    {
        // All node data first
        readNode(f);
        // Then all non-video data
        readMetrics(f);
        // Then all video data
        readData(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readNode(uchar*& data)
    {
        uchar rw = *data++;
        uchar cl = *data++;
        min = (rw << 8) | cl;
        rw = *data++;
        cl = *data++;
        max = (rw << 8) | cl;
        int flags = *data++;
        if ( flags & 1 )
            less = new QPFGlyphTree;
        else
            less = 0;
        if ( flags & 2 )
            more = new QPFGlyphTree;
        else
            more = 0;
        int n = max-min+1;
        glyph = new QPFGlyph[n];

        if ( less )
            less->readNode(data);
        if ( more )
            more->readNode(data);
    }
#else
    void readNode(QIODevice& f)
    {
        uchar rw = f.getch();
        uchar cl = f.getch();
        min = (rw << 8) | cl;
        rw = f.getch();
        cl = f.getch();
        max = (rw << 8) | cl;
        int flags = f.getch();
        if ( flags & 1 )
            less = new QPFGlyphTree;
        else
            less = 0;
        if ( flags & 2 )
            more = new QPFGlyphTree;
        else
            more = 0;
        int n = max-min+1;
        glyph = new QPFGlyph[n];

        if ( less )
            less->readNode(f);
        if ( more )
            more->readNode(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readMetrics(uchar*& data)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            glyph[i].metrics = (QPFGlyphMetrics*)data;
            data += sizeof(QPFGlyphMetrics);
        }
        if ( less )
            less->readMetrics(data);
        if ( more )
            more->readMetrics(data);
    }
#else
    void readMetrics(QIODevice& f)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            glyph[i].metrics = new QPFGlyphMetrics;
            f.readBlock((char*)glyph[i].metrics, sizeof(QPFGlyphMetrics));
        }
        if ( less )
            less->readMetrics(f);
        if ( more )
            more->readMetrics(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readData(uchar*& data)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
            //######### s = qt_screen->mapToDevice( s );
            uint datasize = glyph[i].metrics->linestep * s.height();
            glyph[i].data = data; data += datasize;
        }
        if ( less )
            less->readData(data);
        if ( more )
            more->readData(data);
    }
#else
    void readData(QIODevice& f)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
            //############### s = qt_screen->mapToDevice( s );
            uint datasize = glyph[i].metrics->linestep * s.height();
            glyph[i].data = new uchar[datasize]; // ### deleted?
            f.readBlock((char*)glyph[i].data, datasize);
        }
        if ( less )
            less->readData(f);
        if ( more )
            more->readData(f);
    }
#endif

};

class QFontEngineQPFData
{
public:
    QPFFontMetrics fm;
    QPFGlyphTree *tree;
};


QFontEngineQPF::QFontEngineQPF(const QFontDef&, const QString &fn)
{
    cache_cost = 1;

    int f = ::open( QFile::encodeName(fn), O_RDONLY );
    Q_ASSERT(f>=0);
    struct stat st;
    if ( fstat( f, &st ) )
        qFatal("Failed to stat %s",QFile::encodeName(fn).data());
    uchar* data = (uchar*)mmap( 0, // any address
                                st.st_size, // whole file
                                PROT_READ, // read-only memory
#if !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX4)
                                MAP_FILE | MAP_PRIVATE, // swap-backed map from file
#else
                                MAP_PRIVATE,
#endif
                                f, 0 ); // from offset 0 of f
#if defined(Q_OS_QNX4) && !defined(MAP_FAILED)
#define MAP_FAILED ((void *)-1)
#endif
    if ( !data || data == (uchar*)MAP_FAILED )
        qFatal("Failed to mmap %s",QFile::encodeName(fn).data());
    ::close(f);

    d = new QFontEngineQPFData;
    memcpy(reinterpret_cast<char*>(&d->fm),data,sizeof(d->fm));

    data += sizeof(d->fm);
    d->tree = new QPFGlyphTree(data);

#if 0
    qDebug() << "font file" << fn
             << "ascent" << d->fm.ascent << "descent" << d->fm.descent
             << "leftbearing" << d->fm.leftbearing
             << "rightbearing" << d->fm.rightbearing
             << "maxwidth" << d->fm.maxwidth
             << "leading" << d->fm.leading
             << "flags" << d->fm.flags
             << "underlinepos" << d->fm.underlinepos
             << "underlinewidth" << d->fm.underlinewidth;
#endif
}

QFontEngineQPF::~QFontEngineQPF()
{
}

QFontEngine::FECaps QFontEngineQPF::capabilites() const
{
    return NoTransformations;
}

bool QFontEngineQPF::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    for(int i = 0; i < len; i++)
        glyphs[i].glyph = str[i].unicode();
    *nglyphs = len;

    for(int i = 0; i < len; i++) {
        QGlyphLayout &g=glyphs[i];
        QPFGlyph *glyph = d->tree->get(g.glyph);

        g.advance.x = glyph ? glyph->metrics->advance : 0;
        g.advance.y = 0;
    }

    return true;
}

void QFontEngineQPF::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &si)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);
//###    Q_ASSERT(p->painterState()->txop < QPainterPrivate::TxScale);
    if (1) { //### p->painterState()->txop == QPainterPrivate::TxTranslate) {
        QPointF tmpPt(_x, _y);
        tmpPt = tmpPt * pState->matrix();
        _x = tmpPt.x();
        _y = tmpPt.y();
    }

    QFixed x = QFixed::fromReal(_x);
    QFixed y = QFixed::fromReal(_y);

    if(si.width != 0 && si.flags != 0) {
        int lw = qRound(lineThickness());
        lw = qMax(1, lw);
        if(si.flags & QTextItem::Underline)
            paintEngine->qwsFillRect(qRound(x), qRound(y + underlinePosition()), qRound(si.width), lw);
        if(si.flags & QTextItem::Overline)
            paintEngine->qwsFillRect(qRound(x), qRound(y - (ascent() + 1)), qRound(si.width), lw);
        if(si.flags & QTextItem::StrikeOut)
            paintEngine->qwsFillRect(qRound(x), qRound(y - (ascent() / 3)), qRound(si.width), lw);
    }

    QGlyphLayout *glyphs = si.glyphs;

    if (si.flags & QTextItem::RightToLeft)
        glyphs += si.num_glyphs - 1;

    for(int i = 0; i < si.num_glyphs; i++) {
        const QGlyphLayout *g = glyphs + (si.flags & QTextItem::RightToLeft ? -i : i);
        const QPFGlyph *glyph = d->tree->get(g->glyph);
        if (!glyph)
            continue;
        int myw = glyph->metrics->width;
        int myh = glyph->metrics->height;
        int myx = qRound(x + g->offset.x + glyph->metrics->bearingx);
        int myy = qRound(y + g->offset.y - glyph->metrics->bearingy);


        int mono = !(d->fm.flags & FM_SMOOTH);
        int bpl = glyph->metrics->linestep;

        if(myw != 0 && myh != 0 && bpl != 0) {
            paintEngine->alphaPenBlt(glyph->data, bpl, mono, myx,myy,myw,myh);
        }
        x += qRound(g->advance.x);
    }
}


void QFontEngineQPF::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

glyph_metrics_t QFontEngineQPF::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
   if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineQPF::boundingBox(glyph_t glyph)
{
    const QPFGlyph *g = d->tree->get(glyph);
    Q_ASSERT(g);
    return glyph_metrics_t(g->metrics->bearingx, -g->metrics->bearingy,
                            g->metrics->width, g->metrics->height,
                            g->metrics->advance, 0);
}

QFixed QFontEngineQPF::ascent() const
{
    return d->fm.ascent;
}

QFixed QFontEngineQPF::descent() const
{
    return d->fm.descent;
}

QFixed QFontEngineQPF::leading() const
{
    return d->fm.leading;
}

qreal QFontEngineQPF::maxCharWidth() const
{
    return d->fm.maxwidth;
}
/*
const char *QFontEngineQPF::name() const
{
    return "qt";
}
*/
bool QFontEngineQPF::canRender(const QChar *str, int len)
{
    for(int i = 0; i < len; i++)
        if (!d->tree->inFont(str[i].unicode()))
            return false;
    return true;
}

QFontEngine::Type QFontEngineQPF::type() const
{
    return QPF;
}

qreal QFontEngineQPF::minLeftBearing() const
{
    return d->fm.leftbearing;
}

qreal QFontEngineQPF::minRightBearing() const
{
    return d->fm.rightbearing;
}

QFixed QFontEngineQPF::underlinePosition() const
{
    return d->fm.underlinepos;
}

QFixed QFontEngineQPF::lineThickness() const
{
    return d->fm.underlinewidth;
}


#endif //QT_NO_QWS_QPF
