/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbitmap.h"

// #define FONTENGINE_DEBUG

#include <qapplication.h>
#include <qbytearray.h>
#include <qdebug.h>
#include <qtextcodec.h>
#include <qthread.h>

#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qvarlengtharray.h"
#include "qwidget.h"
#include "qsettings.h"
#include "qfile.h"

#include <private/qpaintengine_x11_p.h>
#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include <qhash.h>

#include <private/qpainter_p.h>
#include <private/qunicodetables_p.h>

#include <private/qt_x11_p.h>
#include "qx11info_x11.h"
#include "qfontengine_x11_p.h"

#include <limits.h>


// ------------------------------------------------------------------
// Multi XLFD engine
// ------------------------------------------------------------------

QFontEngineMultiXLFD::QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s)
    : QFontEngineMulti(l.size()), encodings(l), screen(s), request(r)
{
    loadEngine(0);
    fontDef = engines[0]->fontDef;
}

QFontEngineMultiXLFD::~QFontEngineMultiXLFD()
{ }

void QFontEngineMultiXLFD::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);
    const int encoding = encodings.at(at);
    QFontEngine *fontEngine = QFontDatabase::loadXlfd(0, QUnicodeTables::Common, request, encoding);
    Q_ASSERT(fontEngine != 0);
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}

// ------------------------------------------------------------------
// Xlfd font engine
// ------------------------------------------------------------------

#ifndef QT_NO_FREETYPE

static QStringList *qt_fontpath = 0;

static QStringList fontPath()
{
    if (qt_fontpath)
        return *qt_fontpath;

    // append qsettings fontpath
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    QStringList fontpath;

    int npaths;
    char** font_path;
    font_path = XGetFontPath(X11->display, &npaths);
    bool xfsconfig_read = false;
    for (int i=0; i<npaths; i++) {
        // If we're using xfs, append font paths from /etc/X11/fs/config
        // can't hurt, and chances are we'll get all fonts that way.
        if (((font_path[i])[0] != '/') && !xfsconfig_read) {
            // We're using xfs -> read its config
            bool finished = false;
            QFile f(QLatin1String("/etc/X11/fs/config"));
            if (!f.exists())
                f.setFileName(QLatin1String("/usr/X11R6/lib/X11/fs/config"));
            if (!f.exists())
                f.setFileName(QLatin1String("/usr/X11/lib/X11/fs/config"));
            if (f.exists()) {
                f.open(QIODevice::ReadOnly);
                while (f.error()==QFile::NoError && !finished) {
                    QString fs = QString::fromLocal8Bit(f.readLine(1024));
                    fs=fs.trimmed();
                    if (fs.left(9)==QLatin1String("catalogue") && fs.contains(QLatin1Char('='))) {
                        fs = fs.mid(fs.indexOf(QLatin1Char('=')) + 1).trimmed();
                        bool end = false;
                        while (f.error()==QFile::NoError && !end) {
                            if (fs[int(fs.length())-1] == QLatin1Char(','))
                                fs = fs.left(fs.length()-1);
                            else
                                end = true;

                            fs = fs.left(fs.indexOf(QLatin1String(":unscaled")));
                            if (fs[0] != QLatin1Char('#'))
                                fontpath += fs;
                            fs = QLatin1String(f.readLine(1024));
                            fs = fs.trimmed();
                            if (fs.isEmpty())
                                end = true;
                        }
                        finished = true;
                    }
                }
                f.close();
            }
            xfsconfig_read = true;
        } else {
            QString fs = QString::fromLocal8Bit(font_path[i]);
            fontpath += fs.left(fs.indexOf(QLatin1String(":unscaled")));
        }
    }
    XFreeFontPath(font_path);

    // append qsettings fontpath
    QStringList fp = settings.value(QLatin1String("fontPath")).toStringList();
    if (!fp.isEmpty())
        fontpath += fp;

    qt_fontpath = new QStringList(fontpath);
    return fontpath;
}

static QFontEngine::FaceId fontFile(const QByteArray &_xname, QFreetypeFace **freetype, int *synth)
{
    *freetype = 0;
    *synth = 0;

    QByteArray xname = _xname.toLower();

    int pos = 0;
    int minus = 0;
    while (minus < 5 && (pos = xname.indexOf('-', pos + 1)))
        ++minus;
    QByteArray searchname = xname.left(pos);
    while (minus < 12 && (pos = xname.indexOf('-', pos + 1)))
        ++minus;
    QByteArray encoding = xname.mid(pos + 1);
    //qDebug("xname='%s', searchname='%s', encoding='%s'", xname.data(), searchname.data(), encoding.data());
    QStringList fontpath = ::fontPath();
    QFontEngine::FaceId face_id;
    face_id.index = 0;

    QByteArray best_mapping;

    for (QStringList::ConstIterator it = fontpath.constBegin(); it != fontpath.constEnd(); ++it) {
        if ((*it).left(1) != QLatin1String("/"))
            continue; // not a path name, a font server
        QString fontmapname;
        int num = 0;
        // search font.dir and font.scale for the right file
        while (num < 2) {
            if (num == 0)
                fontmapname = (*it) + QLatin1String("/fonts.scale");
            else
                fontmapname = (*it) + QLatin1String("/fonts.dir");
            ++num;
            //qWarning(fontmapname);
            QFile fontmap(fontmapname);
            if (!fontmap.open(QIODevice::ReadOnly))
                continue;
            while (!fontmap.atEnd()) {
                QByteArray mapping = fontmap.readLine();
                QByteArray lmapping = mapping.toLower();

                //qWarning(xfontname);
                //qWarning(mapping);
                if (!lmapping.contains(searchname))
                    continue;
                int index = mapping.indexOf(' ');
                QByteArray ffn = mapping.mid(0,index);
                // remove bitmap formats freetype can't handle
                if (ffn.contains(".spd") || ffn.contains(".phont"))
                    continue;
                bool best_match = false;
                if (!best_mapping.isEmpty()) {
                    if (lmapping.contains("-0-0-0-0-")) { // scalable font
                        best_match = true;
                        goto found;
                    }
                    if (lmapping.contains(encoding) && !best_mapping.toLower().contains(encoding))
                        goto found;
                    continue;
                }

            found:
                int colon = ffn.lastIndexOf(':');
                if (colon != -1) {
                    QByteArray s = ffn.left(colon);
                    ffn = ffn.mid(colon + 1);
                    if (s.contains("ds="))
                        *synth |= QFontEngine::SynthesizedBold;
                    if (s.contains("ai="))
                        *synth |= QFontEngine::SynthesizedItalic;
                }
                face_id.filename = (*it).toLocal8Bit() + '/' + ffn;
                best_mapping = mapping;
                if (best_match)
                    goto end;
            }
        }
    }
end:
//     qDebug("fontfile for %s is from '%s'\n    got %s synth=%d", xname.data(),
//            best_mapping.data(), face_id.filename.data(), *synth);
    *freetype = QFreetypeFace::getFace(face_id);
    if (!*freetype) {
        face_id.index = 0;
        face_id.filename = QByteArray();
    }
    return face_id;
}

#endif // QT_NO_FREETYPE

// defined in qfontdatabase_x11.cpp
extern int qt_mib_for_xlfd_encoding(const char *encoding);
extern int qt_xlfd_encoding_id(const char *encoding);

static inline XCharStruct *charStruct(XFontStruct *xfs, uint ch)
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if (xfs->per_char &&
         r >= xfs->min_byte1 &&
         r <= xfs->max_byte1 &&
         c >= xfs->min_char_or_byte2 &&
         c <= xfs->max_char_or_byte2) {
        xcs = xfs->per_char + ((r - xfs->min_byte1) *
                               (xfs->max_char_or_byte2 -
                                xfs->min_char_or_byte2 + 1)) +
              (c - xfs->min_char_or_byte2);
        if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
            xcs = 0;
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD(XFontStruct *fs, const QByteArray &name, int mib)
    : _fs(fs), _name(name), _codec(0), _cmap(mib)
{
    if (_cmap) _codec = QTextCodec::codecForMib(_cmap);

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
                   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
                  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
                  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;
    face_id.index = -1;
    freetype = 0;
    synth = 0;
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont(QX11Info::display(), _fs);
    _fs = 0;
#ifndef QT_NO_FREETYPE
    if (freetype)
        freetype->release(face_id);
#endif
}

bool QFontEngineXLFD::stringToCMap(const QChar *s, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    // filter out surrogates, we can't handle them anyway with XLFD fonts
    QVarLengthArray<ushort> _s(len);
    QChar *str = (QChar *)_s.data();
    for (int i = 0; i < len; ++i) {
        if (i < len - 1
            && s[i].unicode() >= 0xd800 && s[i].unicode() < 0xdc00
            && s[i+1].unicode() >= 0xdc00 && s[i].unicode() < 0xe000) {
            *str = QChar();
            ++i;
        } else {
            *str = s[i];
        }
        ++str;
    }

    len = str - (QChar *)_s.data();
    str = (QChar *)_s.data();

    bool mirrored = flags & QTextEngine::RightToLeft;
    if (_codec) {
        bool haveNbsp = false;
        for (int i = 0; i < len; i++)
            if (str[i].unicode() == 0xa0) {
                haveNbsp = true;
                break;
            }

        QVarLengthArray<unsigned short> ch(len);
        QChar *chars = (QChar *)ch.data();
        if (haveNbsp || mirrored) {
            for (int i = 0; i < len; i++)
                chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
                            (mirrored ? QChar::mirroredChar(str[i].unicode()) : str[i].unicode()));
        } else {
            for (int i = 0; i < len; i++)
                chars[i] = str[i].unicode();
        }
        QTextCodec::ConverterState state;
        state.flags = QTextCodec::ConvertInvalidToNull;
        QByteArray ba = _codec->fromUnicode(chars, len, &state);
        if (ba.length() == 2*len) {
            // double byte encoding
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++) {
                glyphs[i].glyph = ((ushort)data[0] << 8) + data[1];
                data += 2;
            }
        } else {
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++)
                glyphs[i].glyph = (ushort)data[i];
        }
    } else {
        QGlyphLayout *g = glyphs + len;
        const QChar *c = str + len;
        if (mirrored) {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : QChar::mirroredChar(c->unicode());
        } else {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
        }
    }
    *nglyphs = len;

    QGlyphLayout *g = glyphs + len;
    XCharStruct *xcs;
    // inlined for better perfomance
    if (!_fs->per_char) {
        xcs = &_fs->min_bounds;
        while (g != glyphs) {
            --g;
            const unsigned char r = g->glyph >> 8;
            const unsigned char c = g->glyph & 0xff;
            if (r >= _fs->min_byte1 &&
                r <= _fs->max_byte1 &&
                c >= _fs->min_char_or_byte2 &&
                c <= _fs->max_char_or_byte2) {
                g->advance.x = xcs->width;
            } else {
                g->glyph = 0;
            }
        }
    }
    else if (!_fs->max_byte1) {
        XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
        while (g != glyphs) {
            unsigned int gl = (--g)->glyph;
            xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
                  base + gl : 0;
            if (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) {
                g->glyph = 0;
            } else {
                g->advance.x = xcs->width;
            }
        }
    }
    else {
        while (g != glyphs) {
            xcs = charStruct(_fs, (--g)->glyph);
            if (!xcs) {
                g->glyph = 0;
            } else {
                g->advance.x = xcs->width;
            }
        }
    }
    return true;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    int i;

    glyph_metrics_t overall;
    // initialize with line height, we get the same behaviour on all platforms
    overall.y = -ascent();
    overall.height = ascent() + descent() + 1;
    QFixed ymax;
    QFixed xmax;
    for (i = 0; i < numGlyphs; i++) {
        XCharStruct *xcs = charStruct(_fs, glyphs[i].glyph);
        if (xcs) {
            QFixed x = overall.xoff + glyphs[i].offset.x + xcs->lbearing;
            QFixed y = overall.yoff + glyphs[i].offset.y - xcs->ascent;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, overall.xoff + glyphs[i].offset.x + xcs->rbearing);
            ymax = qMax(ymax, y + xcs->ascent + xcs->descent);
            overall.xoff += glyphs[i].advance.x;
        } else {
            QFixed size = _fs->ascent;
            overall.x = qMin(overall.x, overall.xoff);
            overall.y = qMin(overall.y, overall.yoff - size);
            ymax = qMax(ymax, overall.yoff);
            overall.xoff += size;
            xmax = qMax(xmax, overall.xoff);
        }
    }
    overall.height = qMax(overall.height, ymax - overall.y);
    overall.width = xmax - overall.x;

    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(glyph_t glyph)
{
    glyph_metrics_t gm;
    XCharStruct *xcs = charStruct(_fs, glyph);
    if (xcs) {
        gm = glyph_metrics_t(xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent,
                              xcs->width, 0);
    } else {
        QFixed size = ascent();
        gm = glyph_metrics_t(0, size, size, size, size, 0);
    }
    return gm;
}

QFixed QFontEngineXLFD::ascent() const
{
    return _fs->ascent;
}

QFixed QFontEngineXLFD::descent() const
{
    return (_fs->descent-1);
}

QFixed QFontEngineXLFD::leading() const
{
    QFixed l = QFixed(qMin<int>(_fs->ascent, _fs->max_bounds.ascent)
                      + qMin<int>(_fs->descent, _fs->max_bounds.descent)) * QFixed::fromReal(0.15);
    return l.ceil();
}

qreal QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
             (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
            f->max_char_or_byte2 - f->min_char_or_byte2);
}

qreal QFontEngineXLFD::minLeftBearing() const
{
    if (lbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->lbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].lbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->lbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return lbearing;
}

qreal QFontEngineXLFD::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->rbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].rbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->rbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return rbearing;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender(const QChar *string, int len)
{
    QVarLengthArray<QGlyphLayout, 256> glyphs(len);
    int nglyphs = len;
    if (stringToCMap(string, len, glyphs.data(), &nglyphs, 0) == false) {
        glyphs.resize(nglyphs);
        stringToCMap(string, len, glyphs.data(), &nglyphs, 0);
    }

    bool allExist = true;
    for (int i = 0; i < nglyphs; i++) {
        if (!glyphs[i].glyph || !charStruct(_fs, glyphs[i].glyph)) {
            allExist = false;
            break;
        }
    }

    return allExist;
}

void QFontEngineXLFD::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

QFontEngine::FaceId QFontEngineXLFD::faceId() const
{
#ifndef QT_NO_FREETYPE
    if (face_id.index == -1) {
        face_id = ::fontFile(_name, &freetype, &synth);
        if (_codec)
            face_id.encoding = _codec->mibEnum();
        if (freetype) {
            const_cast<QFontEngineXLFD *>(this)->fsType = freetype->fsType();
        } else {
            QFontEngine::Properties properties = QFontEngine::properties();
            face_id.index = 0;
            face_id.filename = "-" + properties.postscriptName;
        }
    }
#endif

    return face_id;
}

QFontEngine::Properties QFontEngineXLFD::properties() const
{
    if (face_id.index == -1)
        (void)faceId();

#ifndef QT_NO_FREETYPE
    if (freetype)
        return freetype->properties();
#endif
    return QFontEngine::properties();
}

void QFontEngineXLFD::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    if (face_id.index == -1)
        (void)faceId();
#ifndef QT_NO_FREETYPE
    if (!freetype)
#endif
    {
        QFontEngine::getUnscaledGlyph(glyph, path, metrics);
        return;
    }

#ifndef QT_NO_FREETYPE
    freetype->lock();

    FT_Face face = freetype->face;
    FT_Set_Char_Size(face, face->units_per_EM << 6, face->units_per_EM << 6, 0, 0);
    freetype->xsize = face->units_per_EM << 6;
    freetype->ysize = face->units_per_EM << 6;
    FT_Set_Transform(face, 0, 0);
    glyph = glyphIndexToFreetypeGlyphIndex(glyph);
    FT_Load_Glyph(face, glyph, FT_LOAD_NO_BITMAP);

    int left  = face->glyph->metrics.horiBearingX;
    int right = face->glyph->metrics.horiBearingX + face->glyph->metrics.width;
    int top    = face->glyph->metrics.horiBearingY;
    int bottom = face->glyph->metrics.horiBearingY - face->glyph->metrics.height;

    QFixedPoint p;
    p.x = 0;
    p.y = 0;
    metrics->width = QFixed::fromFixed(right-left);
    metrics->height = QFixed::fromFixed(top-bottom);
    metrics->x = QFixed::fromFixed(left);
    metrics->y = QFixed::fromFixed(-top);
    metrics->xoff = QFixed::fromFixed(face->glyph->advance.x);

    if (!FT_IS_SCALABLE(freetype->face))
        QFreetypeFace::addBitmapToPath(face->glyph, p, path);
    else
        QFreetypeFace::addGlyphToPath(face, face->glyph, p, path, face->units_per_EM << 6, face->units_per_EM << 6);

    FT_Set_Transform(face, &freetype->matrix, 0);
    freetype->unlock();
#endif // QT_NO_FREETYPE
}


QByteArray QFontEngineXLFD::getSfntTable(uint tag) const
{
#ifndef QT_NO_FREETYPE
    if (face_id.index == -1)
        (void)faceId();
    if (!freetype)
        return QByteArray();
    return freetype->getSfntTable(tag);
#else
    Q_UNUSED(tag);
    return QByteArray();
#endif
}

int QFontEngineXLFD::synthesized() const
{
    return synth;
}

#ifndef QT_NO_FREETYPE

FT_Face QFontEngineXLFD::non_locked_face() const
{
    return freetype ? freetype->face : 0;
}

uint QFontEngineXLFD::toUnicode(glyph_t g) const
{
    if (_codec) {
        QTextCodec::ConverterState state;
        state.flags = QTextCodec::ConvertInvalidToNull;
        uchar data[2];
        int l = 1;
        if (g > 255) {
            data[0] = (g >> 8);
            data[1] = (g & 255);
            l = 2;
        } else {
            data[0] = g;
        }
        QString s = _codec->toUnicode((char *)data, l, &state);
        Q_ASSERT(s.length() == 1);
        g = s.at(0).unicode();
    }
    return g;
}

glyph_t QFontEngineXLFD::glyphIndexToFreetypeGlyphIndex(glyph_t g) const
{
    return FT_Get_Char_Index(freetype->face, toUnicode(g));
}
#endif

#ifndef QT_NO_FONTCONFIG

// ------------------------------------------------------------------
// Multi FT engine
// ------------------------------------------------------------------

static QFontEngine *engineForPattern(FcPattern *pattern, const QFontDef &request,
                                     int screen)
{
    FcResult res;
    FcPattern *match = FcFontMatch(0, pattern, &res);
    QFontEngineX11FT *engine = new QFontEngineX11FT(match, request, screen);
    if (!engine->invalid())
        return engine;

    delete engine;
    QFontEngine *fe = new QFontEngineBox(request.pixelSize);
    fe->fontDef = request;
    return fe;
}

QFontEngineMultiFT::QFontEngineMultiFT(QFontEngine *fe, FcPattern *p, int s, const QFontDef &req)
    : QFontEngineMulti(2), request(req), pattern(p), fontSet(0), screen(s)
{

    engines[0] = fe;
    engines.at(0)->ref.ref();
    fontDef = engines[0]->fontDef;
    cache_cost = 100;
}

QFontEngineMultiFT::~QFontEngineMultiFT()
{
    FcPatternDestroy(pattern);
    if (fontSet)
        FcFontSetDestroy(fontSet);
}


void QFontEngineMultiFT::loadEngine(int at)
{
    extern void qt_addPatternProps(FcPattern *pattern, int screen, int script,
                                   const QFontDef &request);
    extern QFontDef qt_FcPatternToQFontDef(FcPattern *pattern, const QFontDef &);
    extern FcFontSet *qt_fontSetForPattern(FcPattern *pattern, const QFontDef &request);

    Q_ASSERT(at > 0);
    if (!fontSet) {
        fontSet = qt_fontSetForPattern(pattern, request);
        engines.resize(fontSet->nfont);
    }
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);

    FcPattern *pattern = FcPatternDuplicate(fontSet->fonts[at]);
    qt_addPatternProps(pattern, screen, QUnicodeTables::Common, request);

    QFontDef fontDef = qt_FcPatternToQFontDef(pattern, this->request);

    // note: we use -1 for the script to make sure that we keep real
    // FT engines separate from Multi engines in the font cache
    QFontCache::Key key(fontDef, -1, screen);
    QFontEngine *fontEngine = QFontCache::instance->findEngine(key);
    if (!fontEngine) {
        FcConfigSubstitute(0, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);
        fontEngine = engineForPattern(pattern, request, screen);
        FcPatternDestroy(pattern);
        QFontCache::instance->insertEngine(key, fontEngine);
    }
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}

// ------------------------------------------------------------------
// X11 FT engine
// ------------------------------------------------------------------



Q_GUI_EXPORT void qt_x11ft_convert_pattern(FcPattern *pattern, QByteArray *file_name, int *index, bool *antialias)
{
    FcChar8 *fileName;
    FcPatternGetString(pattern, FC_FILE, 0, &fileName);
    *file_name = (const char *)fileName;
    if (!FcPatternGetInteger(pattern, FC_INDEX, 0, index))
        index = 0;
    FcBool b;
    if (FcPatternGetBool(pattern, FC_ANTIALIAS, 0, &b) == FcResultMatch)
        *antialias = b;
}


QFontEngineX11FT::QFontEngineX11FT(FcPattern *pattern, const QFontDef &fd, int screen)
    : QFontEngineFT(fd)
{
//     FcPatternPrint(pattern);

    bool antialias = X11->fc_antialias;
    QByteArray file_name;
    int face_index;
    qt_x11ft_convert_pattern(pattern, &file_name, &face_index, &antialias);
    QFontEngine::FaceId face_id;
    face_id.filename = file_name;
    face_id.index = face_index;

    canUploadGlyphsToServer = qApp->thread() == QThread::currentThread();

    subpixelType = Subpixel_None;
    if (antialias) {
        int subpixel = 0;
        if (FcPatternGetInteger(pattern, FC_RGBA, 0, &subpixel) == FcResultNoMatch
            && X11->display)
            subpixel = X11->screens[screen].subpixel;
        if (!antialias || subpixel == FC_RGBA_UNKNOWN)
            subpixel = FC_RGBA_NONE;

        switch (subpixel) {
            case FC_RGBA_NONE: subpixelType = Subpixel_None; break;
            case FC_RGBA_RGB: subpixelType = Subpixel_RGB; break;
            case FC_RGBA_BGR: subpixelType = Subpixel_BGR; break;
            case FC_RGBA_VRGB: subpixelType = Subpixel_VRGB; break;
            case FC_RGBA_VBGR: subpixelType = Subpixel_VBGR; break;
            default: break;
        }
    }

#ifdef FC_HINT_STYLE
    {
        int hint_style = 0;
        if (FcPatternGetInteger (pattern, FC_HINT_STYLE, 0, &hint_style) == FcResultNoMatch)
            hint_style = X11->fc_hint_style;
        if (hint_style == FC_HINT_NONE)
            default_load_flags |= FT_LOAD_NO_HINTING;
        else if (hint_style < FC_HINT_FULL)
            default_load_flags |= FT_LOAD_TARGET_LIGHT;
    }
#endif

#if defined(FC_AUTOHINT) && defined(FT_LOAD_FORCE_AUTOHINT)
    {
        bool autohint = false;

        FcBool b;
        if (FcPatternGetBool(pattern, FC_AUTOHINT, 0, &b) == FcResultMatch)
            autohint = b;

        if (autohint)
            default_load_flags |= FT_LOAD_FORCE_AUTOHINT;
    }
#endif

    GlyphFormat defaultFormat = Format_None;

#ifndef QT_NO_XRENDER
    if (X11->use_xrender) {
        int format = PictStandardA8;
        if (!antialias)
            format = PictStandardA1;
        else if (subpixelType == Subpixel_RGB
                 || subpixelType == Subpixel_BGR
                 || subpixelType == Subpixel_VRGB
                 || subpixelType == Subpixel_VBGR)
            format = PictStandardARGB32;
        xglyph_format = format;

        if (subpixelType != QFontEngineFT::Subpixel_None)
            defaultFormat = Format_A32;
        else if (antialias)
            defaultFormat = Format_A8;
        else
            defaultFormat = Format_Mono;
    }
#endif

    if (!init(face_id, antialias, defaultFormat)) {
        FcPatternDestroy(pattern);
        return;
    }

    if (!freetype->charset) {
        FcCharSet *cs;
        FcPatternGetCharSet (pattern, FC_CHARSET, 0, &cs);
        freetype->charset = FcCharSetCopy(cs);
    }
    FcPatternDestroy(pattern);
}

QFontEngineX11FT::~QFontEngineX11FT()
{
    freeGlyphSets();
}

unsigned long QFontEngineX11FT::allocateServerGlyphSet()
{
#ifndef QT_NO_XRENDER
    if (!canUploadGlyphsToServer || !X11->use_xrender)
        return 0;
    return XRenderCreateGlyphSet(X11->display, XRenderFindStandardFormat(X11->display, xglyph_format));
#else
    return 0;
#endif
}

void QFontEngineX11FT::freeServerGlyphSet(unsigned long id)
{
#ifndef QT_NO_XRENDER
    if (!id)
        return;
    XRenderFreeGlyphSet(X11->display, id);
#endif
}

bool QFontEngineX11FT::uploadGlyphToServer(QGlyphSet *set, uint glyphid, Glyph *g, GlyphInfo *info, int glyphDataSize) const
{
#ifndef QT_NO_XRENDER
    if (!canUploadGlyphsToServer)
        return false;
    if (g->format == Format_Mono) {
        /*
         * swap bit order around; FreeType is always MSBFirst
         */
        if (BitmapBitOrder(X11->display) != MSBFirst) {
            unsigned char *line = g->data;
            int i = glyphDataSize;
            while (i--) {
                unsigned char c;
                c = *line;
                c = ((c << 1) & 0xaa) | ((c >> 1) & 0x55);
                c = ((c << 2) & 0xcc) | ((c >> 2) & 0x33);
                c = ((c << 4) & 0xf0) | ((c >> 4) & 0x0f);
                *line++ = c;
            }
        }
    }

    ::Glyph xglyph = glyphid;
    XRenderAddGlyphs (X11->display, set->id, &xglyph, info, 1, (const char *)g->data, glyphDataSize);
    delete [] g->data;
    g->data = 0;
    g->format = Format_None;
    g->uploadedToServer = true;
    return true;
#else
    return false;
#endif
}

#endif // QT_NO_FONTCONFIG
