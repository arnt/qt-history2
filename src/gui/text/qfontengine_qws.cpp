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
#include <private/qpaintengine_qws_p.h>
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
    _scale =  1;

    smooth = FT_IS_SCALABLE(face);
    if (fontDef.styleStrategy & QFont::NoAntialias)
        smooth = false;
    rendered_glyphs = new QGlyph *[face->num_glyphs];
    memset(rendered_glyphs, 0, face->num_glyphs*sizeof(QGlyph *));
    cache_cost = face->num_glyphs*6*8; // ##########
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
            if (uc == 0xa0) uc = 0x20;
            glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
            ++glyph_pos;
        }
    } else {
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = getChar(str, i, len);
            if (uc == 0xa0) uc = 0x20;
            glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
            ++glyph_pos;
        }
    }
    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);
    glyph_pos = 0;
    return true;
}


void QFontEngineFT::draw(QPaintEngine *p, int x, int y, const QTextItemInt &si)
{
    Q_ASSERT(p->painterState()->txop < QPainterPrivate::TxScale);
    if (p->painterState()->txop == QPainterPrivate::TxTranslate) {
        QPoint tmpPt(x, y);
        tmpPt = tmpPt * p->painterState()->matrix;
        x = tmpPt.x();
        y = tmpPt.y();
    }
    QWSPaintEngine *qpe = static_cast<QWSPaintEngine*>(p);

    if (si.flags) {
        int lw = qRound(lineThickness());
        lw = qMax(1, lw);

        p->updateBrush(p->painterState()->pen.color(), QPoint(0,0));

        if (si.flags & QTextItem::Underline)
            qpe->fillRect(x, y+qRound(underlinePosition()), qRound(si.width), lw);
        if (si.flags & QTextItem::StrikeOut)
            qpe->fillRect(x, y-qRound(ascent())/3, qRound(si.width), lw);
        if (si.flags & QTextItem::Overline)
            qpe->fillRect(x, y-qRound(ascent())-1, qRound(si.width), lw);

        p->updateBrush(p->painterState()->brush, p->painterState()->bgOrigin);
    }

    QGlyphLayout *glyphs = si.glyphs;

#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText grab");
#endif

#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//######## verify that we really need this!!!
    QWSDisplay::grab(); // we need it later, and grab-must-precede-lock
#endif

#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText lock");
#endif


    if (si.flags & QTextItem::RightToLeft)
        glyphs += si.num_glyphs - 1;
    for(int i = 0; i < si.num_glyphs; i++) {
        const QGlyphLayout *g = glyphs + (si.flags & QTextItem::RightToLeft ? -i : i);
        const QGlyph *glyph = rendered_glyphs[g->glyph];
        if (!glyph) {
            FT_UInt gi = glyphs[i].glyph;
            glyph = rendered_glyphs[gi] = new QGlyph;
            render(face, gi, rendered_glyphs[gi], smooth);
        }
        int myw = glyph->width;
        int myx = x + qRound(g->offset.x() + glyph->bearingx);
        int myy = y + qRound(g->offset.y() - glyph->bearingy);

        if(glyph->width != 0 && glyph->height != 0 && glyph->pitch != 0)
            qpe->alphaPenBlt(glyph->data, glyph->pitch, glyph->mono, myx,myy,myw,glyph->height,0,0);

        x += qRound(g->advance.x());
    }
#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText unlock");
#endif
#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText ungrab");
#endif
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
    QWSDisplay::ungrab();
#endif
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    qreal w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x();
    w *= _scale;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    const QGlyph *g = rendered_glyphs[glyph];
    Q_ASSERT(g);
    return glyph_metrics_t(g->bearingx*_scale, g->bearingy*_scale,
                            g->width*_scale, g->height*_scale,
                            g->advance*_scale, 0);
}

static void addCurve(QPainterPath *path, const QPointF &cp, const QPointF &endPoint,
                     int startOff, int nOff, FT_GlyphSlot g)
{
    int j;
    QPointF c0 = QPointF(g->outline.points[startOff-1].x/64., -g->outline.points[startOff-1].y/64.);
    QPointF current = QPointF(g->outline.points[startOff].x/64., -g->outline.points[startOff].y/64.);
    for(j = 1; j <= nOff; j++) {
        QPointF next = (j == nOff)
                       ? endPoint
                       : QPointF(g->outline.points[startOff + j].x/64., -g->outline.points[startOff + j].y/64.);
        QPointF c3 = (j == nOff) ? next : (next + current)/2;
        QPointF c1 = (2*current + c0)/3;
        QPointF c2 = (2*current + c3)/3;
//         qDebug("cubicTo %f/%f %f/%f %f/%f", (cp + c1).x(),  (cp + c1).y(),
//                (cp + c2).x(),  (cp + c2).y(), (cp + c3).x(),  (cp + c3).y());
        path->cubicTo(cp + c1, cp + c2, cp + c3);
        c0 = c3;
        current = next;
    }
}

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path)
{
    if (FT_IS_SCALABLE(face)) {
        QPointF point = QPointF(x, y);
        for (int i = 0; i < numGlyphs; i++) {
            FT_UInt glyph = glyphs[i].glyph;
            QPointF cp = point + glyphs[i].offset;
            point += glyphs[i].advance;

            FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);

            FT_GlyphSlot g = face->glyph;
            if (g->format != FT_GLYPH_FORMAT_OUTLINE)
                continue;

            // convert the outline to a painter path
            int i = 0;
            for (int c = 0; c < g->outline.n_contours; ++c) {
                QPointF p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
                 //qDebug("contour: %d -- %d", i, g->outline.contours[c]);
                 //qDebug("first point at %f %f", p.x(), p.y());
                path->moveTo(p);

                int first = i;
                int startOff = 0;
                int nOff = 0;
                ++i;
                while (i <= g->outline.contours[c]) {
                    QPointF p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
//                     qDebug("     point at %f %f, on curve=%d", p.x(), p.y(), g->outline.tags[i] & 1);
                    if (!(g->outline.tags[i] & 1)) {
                        /* Off curve */
                        if (!startOff) {
                            startOff = i;
                            nOff = 1;
                        } else {
                            ++nOff;
                        }
                    } else {
                        /* On Curve */
                        if (startOff) {
                            // ###### fix 3rd order beziers
                            addCurve(path, cp, QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.),
                                     startOff, nOff, g);
                            startOff = 0;
                        } else {
                            p = cp + QPointF(g->outline.points[i].x/64., -g->outline.points[i].y/64.);
                            path->lineTo(p);
                        }
                    }
                    ++i;
                }
                QPointF end(g->outline.points[first].x/64., -g->outline.points[first].y/64.);
                if (startOff)
                    addCurve(path, cp, end, startOff, nOff, g);
                else
                    path->lineTo(end + cp);
            }
        }
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

qreal QFontEngineFT::ascent() const
{
    return face->size->metrics.ascender/64.;
}

qreal QFontEngineFT::descent() const
{
    return -face->size->metrics.descender/64.;
}

qreal QFontEngineFT::leading() const
{
    return (face->size->metrics.height
            - face->size->metrics.ascender /*ascent*/
            + face->size->metrics.descender)/64.;
}

qreal QFontEngineFT::maxCharWidth() const
{
    return face->size->metrics.max_advance/64.;
}

qreal QFontEngineFT::minLeftBearing() const
{
    return 0;
//     return (memorymanager->fontMinLeftBearing(handle())*_scale)>>8;
}

qreal QFontEngineFT::minRightBearing() const
{
    return 0;
//     return (memorymanager->fontMinRightBearing(handle())*_scale)>>8;
}

qreal QFontEngineFT::underlinePosition() const
{
    return FT_MulFix(face->underline_position, face->size->metrics.y_scale)/64.;
}

qreal QFontEngineFT::lineThickness() const
{
    return FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)/64.;
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
            glyphs[i].advance.rx() = face->glyph->metrics.horiAdvance/qreal(64);
            glyphs[i].advance.ry() = 0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            FT_UInt g = glyphs[i].glyph;
            if (!rendered_glyphs[g]) {
                rendered_glyphs[g] = new QGlyph;
                render(face, g, rendered_glyphs[g], smooth);
            }
            glyphs[i].advance.rx() = (rendered_glyphs[g]->advance);//*_scale)>>8;
        glyphs[i].advance.ry() = 0;
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
            glyphs[i].advance.rx() += kerning.x / qreal(64);
            glyphs[i].advance.ry() += kerning.y / qreal(64);
        }
    }
}

qreal QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

qreal QFontEngine::underlinePosition() const
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

        g.advance.rx() = glyph->metrics->advance;
        g.advance.ry() = 0.;
    }

    return true;
}

void QFontEngineQPF::draw(QPaintEngine *p, int x, int y, const QTextItemInt &si)
{
    Q_ASSERT(p->painterState()->txop < QPainterPrivate::TxScale);
    if (p->painterState()->txop == QPainterPrivate::TxTranslate) {
        QPoint tmpPt(x, y);
        tmpPt = tmpPt * p->painterState()->matrix;
        x = tmpPt.x();
        y = tmpPt.y();
    }
    QWSPaintEngine *qpe = static_cast<QWSPaintEngine*>(p);

    if (si.flags) {
        int lw = qRound(lineThickness());
        lw = qMax(1, lw);

        p->updateBrush(p->painterState()->pen.color(), QPoint(0,0));

        if (si.flags & QTextItem::Underline)
            qpe->fillRect(x, y+qRound(underlinePosition()), qRound(si.width), lw);
        if (si.flags & QTextItem::StrikeOut)
            qpe->fillRect(x, y-qRound(ascent())/3, qRound(si.width), lw);
        if (si.flags & QTextItem::Overline)
            qpe->fillRect(x, y-qRound(ascent())-1, qRound(si.width), lw);

        p->updateBrush(p->painterState()->brush, p->painterState()->bgOrigin);
    }

    QGlyphLayout *glyphs = si.glyphs;

    if (si.flags & QTextItem::RightToLeft)
        glyphs += si.num_glyphs - 1;

    for(int i = 0; i < si.num_glyphs; i++) {
        const QGlyphLayout *g = glyphs + (si.flags & QTextItem::RightToLeft ? -i : i);
        const QPFGlyph *glyph = d->tree->get(g->glyph);
        Q_ASSERT(glyph);
        int myw = glyph->metrics->width;
        int myh = glyph->metrics->height;
        int myx = x + qRound(g->offset.x() + glyph->metrics->bearingx);
        int myy = y + qRound(g->offset.y() - glyph->metrics->bearingy);


        int mono = !(d->fm.flags & FM_SMOOTH);
        int bpl = glyph->metrics->linestep;

        if(myw != 0 && myh != 0 && bpl != 0)
            qpe->alphaPenBlt(glyph->data, bpl, mono, myx,myy,myw,myh,0,0);

        x += qRound(g->advance.x());
    }

    //##### grab/ungrab
}

glyph_metrics_t QFontEngineQPF::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
   if (numGlyphs == 0)
        return glyph_metrics_t();

    qreal w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x();
    w *= _scale;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineQPF::boundingBox(glyph_t glyph)
{
    const QPFGlyph *g = d->tree->get(glyph);
    Q_ASSERT(g);
    return glyph_metrics_t(g->metrics->bearingx, g->metrics->bearingy,
                            g->metrics->width, g->metrics->height,
                            g->metrics->advance, 0);
}

qreal QFontEngineQPF::ascent() const
{
    return d->fm.ascent;
}

qreal QFontEngineQPF::descent() const
{
    return d->fm.descent;
}

qreal QFontEngineQPF::leading() const
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

qreal QFontEngineQPF::underlinePosition() const
{
    return d->fm.underlinepos;
}

qreal QFontEngineQPF::lineThickness() const
{
    return d->fm.underlinewidth;
}


#endif //QT_NO_QWS_QPF
