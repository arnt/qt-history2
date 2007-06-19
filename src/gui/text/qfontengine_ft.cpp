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

#include "qdir.h"
#include "qmetatype.h"
#include "qtextstream.h"
#include "qvariant.h"
#include "qfontengine_ft_p.h"

#ifndef QT_NO_FREETYPE

#include "qfile.h"
#include "qabstractfileengine.h"
#include "qopentype_p.h"
#include <private/qpdf_p.h>
#include <private/qmath_p.h>

#include "qfontengine_ft_p.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H
#include FT_GLYPH_H

/*
 * Freetype 2.1.7 and earlier used width/height
 * for matching sizes in the BDF and PCF loaders.
 * This has been fixed for 2.1.8.
 */
#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
#define X_SIZE(face,i) ((face)->available_sizes[i].x_ppem)
#define Y_SIZE(face,i) ((face)->available_sizes[i].y_ppem)
#else
#define X_SIZE(face,i) ((face)->available_sizes[i].width << 6)
#define Y_SIZE(face,i) ((face)->available_sizes[i].height << 6)
#endif

#define FLOOR(x)    ((x) & -64)
#define CEIL(x)	    (((x)+63) & -64)
#define TRUNC(x)    ((x) >> 6)
#define ROUND(x)    (((x)+32) & -64)

// -------------------------- Freetype support ------------------------------

static FT_Library library = 0;
FT_Library qt_getFreetype()
{
    if (!library)
        FT_Init_FreeType(&library);
    return library;
}
static QHash<QFontEngine::FaceId, QFreetypeFace *> *freetypeFaces = 0;

int QFreetypeFace::fsType() const
{
    int fsType = 0;
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2)
        fsType = os2->fsType;
    return fsType;
}

QFreetypeFace *QFreetypeFace::getFace(const QFontEngine::FaceId &face_id)
{
    if (face_id.filename.isEmpty())
        return 0;

    if (!library)
        FT_Init_FreeType(&library);
    if (!freetypeFaces)
        freetypeFaces = new QHash<QFontEngine::FaceId, QFreetypeFace *>();

    QFreetypeFace *freetype = freetypeFaces->value(face_id, 0);
    if (!freetype) {
        freetype = new QFreetypeFace;
        FT_Face face;
        QFile file(QString::fromUtf8(face_id.filename));
        if (face_id.filename.startsWith(":qmemoryfonts/")) {
            // from qfontdatabase.cpp
            extern QByteArray qt_fontdata_from_index(int);
            QByteArray idx = face_id.filename;
            idx.remove(0, 14); // remove ':qmemoryfonts/'
            bool ok = false;
            freetype->fontData = qt_fontdata_from_index(idx.toInt(&ok));
            if (!ok)
                freetype->fontData = QByteArray();
        } else if (!(file.fileEngine()->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::LocalDiskFlag)) {
            if (!file.open(QIODevice::ReadOnly)) {
                delete freetype;
                return 0;
            }
            freetype->fontData = file.readAll();
        }
        if (!freetype->fontData.isEmpty()) {
            if (FT_New_Memory_Face(library, (const FT_Byte *)freetype->fontData.constData(), freetype->fontData.size(), face_id.index, &face)) {
                delete freetype;
                return 0;
            }
        } else if (FT_New_Face(library, face_id.filename, face_id.index, &face)) {
            delete freetype;
            return 0;
        }
        freetype->face = face;
        freetype->ref = 0;
        freetype->xsize = 0;
        freetype->ysize = 0;
        freetype->matrix.xx = 0x10000;
        freetype->matrix.yy = 0x10000;
        freetype->matrix.xy = 0;
        freetype->matrix.yx = 0;
        freetype->unicode_map = 0;
        freetype->symbol_map = 0;
#ifndef QT_NO_FONTCONFIG
        freetype->charset = 0;
#endif

        memset(freetype->cmapCache, 0, sizeof(freetype->cmapCache));

        for (int i = 0; i < freetype->face->num_charmaps; ++i) {
            FT_CharMap cm = freetype->face->charmaps[i];
            switch(cm->encoding) {
            case ft_encoding_unicode:
                freetype->unicode_map = cm;
                break;
            case ft_encoding_apple_roman:
            case ft_encoding_latin_1:
                if (!freetype->unicode_map || freetype->unicode_map->encoding != ft_encoding_unicode)
                    freetype->unicode_map = cm;
                break;
            case ft_encoding_adobe_custom:
            case ft_encoding_symbol:
                if (!freetype->symbol_map)
                    freetype->symbol_map = cm;
                break;
            default:
                break;
            }
        }

        if (!FT_IS_SCALABLE(freetype->face) && freetype->face->num_fixed_sizes == 1)
            FT_Set_Char_Size (face, X_SIZE(freetype->face, 0), Y_SIZE(freetype->face, 0), 0, 0);
# if 0
        FcChar8 *name;
        FcPatternGetString(pattern, FC_FAMILY, 0, &name);
        qDebug("%s: using maps: default: %x unicode: %x, symbol: %x", name,
               freetype->face->charmap ? freetype->face->charmap->encoding : 0,
               freetype->unicode_map ? freetype->unicode_map->encoding : 0,
               freetype->symbol_map ? freetype->symbol_map->encoding : 0);

        for (int i = 0; i < 256; i += 8)
            qDebug("    %x: %d %d %d %d %d %d %d %d", i,
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i));
#endif

        FT_Set_Charmap(freetype->face, freetype->unicode_map);
        freetypeFaces->insert(face_id, freetype);
    }
    freetype->ref.ref();
    return freetype;
}

void QFreetypeFace::release(const QFontEngine::FaceId &face_id)
{
    if (!ref.deref()) {
        FT_Done_Face(face);
#ifndef QT_NO_FONTCONFIG
        if (charset)
            FcCharSetDestroy(charset);
#endif
        freetypeFaces->take(face_id);
        delete this;
    }
    if (!freetypeFaces->size()) {
        delete freetypeFaces;
        freetypeFaces = 0;
        FT_Done_FreeType(library);
        library = 0;
    }
}


void QFreetypeFace::computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing)
{
    *ysize = fontDef.pixelSize << 6;
    *xsize = *ysize * fontDef.stretch / 100;
    *outline_drawing = false;

    /*
     * Bitmap only faces must match exactly, so find the closest
     * one (height dominant search)
     */
    if (!(face->face_flags & FT_FACE_FLAG_SCALABLE)) {
        int best = 0;
        for (int i = 1; i < face->num_fixed_sizes; i++) {
            if (qAbs(*ysize -  Y_SIZE(face,i)) <
                qAbs (*ysize - Y_SIZE(face, best)) ||
                (qAbs (*ysize - Y_SIZE(face, i)) ==
                 qAbs (*ysize - Y_SIZE(face, best)) &&
                 qAbs (*xsize - X_SIZE(face, i)) <
                 qAbs (*xsize - X_SIZE(face, best)))) {
                best = i;
            }
        }
        if (FT_Set_Char_Size (face, X_SIZE(face, best), Y_SIZE(face, best), 0, 0) == 0) {
            *xsize = X_SIZE(face, best);
            *ysize = Y_SIZE(face, best);
        } else {
            int err = 1;
            if (!(face->face_flags & FT_FACE_FLAG_SCALABLE) && ysize == 0 && face->num_fixed_sizes >= 1) {
                // work around FT 2.1.10 problem with BDF without PIXEL_SIZE property
                err = FT_Set_Pixel_Sizes(face, face->available_sizes[0].width, face->available_sizes[0].height);
                if (err && face->num_fixed_sizes == 1)
                    err = 0; //even more of a workaround...
            }

            if (err)
                *xsize = *ysize = 0;
        }
    } else {
        *outline_drawing = (*xsize > (64<<6) || *ysize > (64<<6));
    }
}

QFontEngine::Properties QFreetypeFace::properties() const
{
    QFontEngine::Properties p;
    p.postscriptName = FT_Get_Postscript_Name(face);
    PS_FontInfoRec font_info;
    if (FT_Get_PS_Font_Info(face, &font_info) == 0)
        p.copyright = font_info.notice;
    if (FT_IS_SCALABLE(face)) {
        p.ascent = face->ascender;
        p.descent = -face->descender;
        p.leading = face->height - face->ascender + face->descender;
        p.emSquare = face->units_per_EM;
        p.boundingBox = QRectF(face->bbox.xMin, -face->bbox.yMax,
                               face->bbox.xMax - face->bbox.xMin,
                               face->bbox.yMax - face->bbox.yMin);
    } else {
        p.ascent = QFixed::fromFixed(face->size->metrics.ascender);
        p.descent = QFixed::fromFixed(-face->size->metrics.descender);
        p.leading = QFixed::fromFixed(face->size->metrics.height - face->size->metrics.ascender + face->size->metrics.descender);
        p.emSquare = face->size->metrics.y_ppem;
        p.boundingBox = QRectF(-p.ascent.toReal(), 0, (p.ascent + p.descent).toReal(), face->size->metrics.max_advance/64.);
    }
    p.italicAngle = 0;
    p.capHeight = p.ascent;
    p.lineWidth = face->underline_thickness;
    return p;
}

QByteArray QFreetypeFace::getSfntTable(uint tag) const
{
    QByteArray table;
#if (FREETYPE_MAJOR*10000 + FREETYPE_MINOR*100 + FREETYPE_PATCH) > 20103
    if (FT_IS_SFNT(face)) {
        FT_ULong length = 0;
        FT_Load_Sfnt_Table(face, tag, 0, 0, &length);
        if (length != 0) {
            table.resize(length);
            FT_Load_Sfnt_Table(face, tag, 0, (FT_Byte *)table.data(), &length);
        }
    }
#endif
    return table;
}

/* Some fonts (such as MingLiu rely on hinting to scale different
   components to their correct sizes. While this is really broken (it
   should be done in the component glyph itself, not the hinter) we
   will have to live with it.

   This means we can not use FT_LOAD_NO_HINTING to get the glyph
   outline. All we can do is to load the unscaled glyph and scale it
   down manually when required.
*/
static void scaleOutline(FT_Face face, FT_GlyphSlot g, FT_Fixed x_scale, FT_Fixed y_scale)
{
    x_scale = FT_MulDiv(x_scale, 1 << 10, face->units_per_EM);
    y_scale = FT_MulDiv(y_scale, 1 << 10, face->units_per_EM);
    FT_Vector *p = g->outline.points;
    const FT_Vector *e = p + g->outline.n_points;
    while (p < e) {
        p->x = FT_MulFix(p->x, x_scale);
        p->y = FT_MulFix(p->y, y_scale);
        ++p;
    }
}

void QFreetypeFace::addGlyphToPath(FT_Face face, FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path, FT_Fixed x_scale, FT_Fixed y_scale)
{
    const qreal factor = 1/64.;
    scaleOutline(face, g, x_scale, y_scale);

    QPointF cp = point.toPointF();

    // convert the outline to a painter path
    int i = 0;
    for (int j = 0; j < g->outline.n_contours; ++j) {
        int last_point = g->outline.contours[j];
        QPointF start = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
        if(!(g->outline.tags[i] & 1)) {
            start += cp + QPointF(g->outline.points[last_point].x*factor, -g->outline.points[last_point].y*factor);
            start /= 2;
        }
//         qDebug("contour: %d -- %d", i, g->outline.contours[j]);
//         qDebug("first point at %f %f", start.x(), start.y());
        path->moveTo(start);

        QPointF c[4];
        c[0] = start;
        int n = 1;
        while (i < last_point) {
            ++i;
            c[n] = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
//             qDebug() << "    i=" << i << " flag=" << (int)g->outline.tags[i] << "point=" << c[n];
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
//                     qDebug() << "lineTo" << c[1];
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
//             qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
            c[0] = c[3];
            n = 1;
        }
        if (n == 1) {
//             qDebug() << "closeSubpath";
            path->closeSubpath();
        } else {
            c[3] = start;
            if (n == 2) {
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
            }
//             qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
        }
        ++i;
    }
}

extern void qt_addBitmapToPath(qreal x0, qreal y0, const uchar *image_data, int bpl, int w, int h, QPainterPath *path);

void QFreetypeFace::addBitmapToPath(FT_GlyphSlot slot, const QFixedPoint &point, QPainterPath *path, bool)
{
    if (slot->format != FT_GLYPH_FORMAT_BITMAP
        || slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO)
        return;

    QPointF cp = point.toPointF();
    qt_addBitmapToPath(cp.x() + TRUNC(slot->metrics.horiBearingX), cp.y() - TRUNC(slot->metrics.horiBearingY),
                       slot->bitmap.buffer, slot->bitmap.pitch, slot->bitmap.width, slot->bitmap.rows, path);
}

QFontEngineFT::Glyph::~Glyph()
{
    delete [] data;
}

static const uint subpixel_filter[3][3] = {
    { 180, 60, 16 },
    { 38, 180, 38 },
    { 16, 60, 180 }
};

QFontEngineFT::QFontEngineFT(const QFontDef &fd)
{
    _openType = 0;
    fontDef = fd;
    matrix.xx = 0x10000;
    matrix.yy = 0x10000;
    matrix.xy = 0;
    matrix.yx = 0;
    cache_cost = 100;
    kerning_pairs_loaded = false;
    transform = false;
    antialias = true;
    default_load_flags = 0;
    subpixelType = Subpixel_None;
    defaultGlyphFormat = Format_None;
    canUploadGlyphsToServer = false;
}

QFontEngineFT::~QFontEngineFT()
{
#ifndef QT_NO_OPENTYPE
    delete _openType;
    _openType = 0;
#endif
    if (freetype)
        freetype->release(face_id);
}

void QFontEngineFT::freeGlyphSets()
{
    freeServerGlyphSet(defaultGlyphSet.id);
    for (int i = 0; i < transformedGlyphSets.count(); ++i)
        freeServerGlyphSet(transformedGlyphSets.at(i).id);
}

bool QFontEngineFT::init(FaceId faceId, bool antialias, GlyphFormat defaultFormat)
{
    defaultGlyphFormat = defaultFormat;
    this->antialias = antialias;
    face_id = faceId;
    freetype = QFreetypeFace::getFace(face_id);
    if (!freetype) {
        xsize = 0;
        ysize = 0;
        return false;
    }
    symbol = freetype->symbol_map != 0;
    PS_FontInfoRec psrec;
    // don't assume that type1 fonts are symbol fonts by default
    if (FT_Get_PS_Font_Info(freetype->face, &psrec) == FT_Err_Ok) {
        symbol = bool(fontDef.family.contains(QLatin1String("symbol"), Qt::CaseInsensitive));
    }

    lbearing = rbearing = SHRT_MIN;
    freetype->computeSize(fontDef, &xsize, &ysize, &outline_drawing);

    FT_Face face = lockFace();

    //underline metrics
    if (FT_IS_SCALABLE(face)) {
        line_thickness =  QFixed::fromFixed(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale));
        underline_position = QFixed::fromFixed(-FT_MulFix(face->underline_position, face->size->metrics.y_scale));
        bool fake_oblique = (fontDef.style != QFont::StyleNormal) && !(face->style_flags & FT_STYLE_FLAG_ITALIC);
        if (fake_oblique)
            matrix.xy = 0x10000*3/10;
        FT_Set_Transform(face, &matrix, 0);
        if (fake_oblique)
            transform = true;
    } else {
        // copied from QFontEngineQPF
        // ad hoc algorithm
        int score = fontDef.weight * fontDef.pixelSize;
        line_thickness = score / 700;
        // looks better with thicker line for small pointsizes
        if (line_thickness < 2 && score >= 1050)
            line_thickness = 2;
        underline_position =  ((line_thickness * 2) + 3) / 6;
    }
    if (line_thickness < 1)
        line_thickness = 1;

    metrics = face->size->metrics;

    unlockFace();

    fsType = freetype->fsType();
    defaultGlyphSet.id = allocateServerGlyphSet();
    return true;
}

QFontEngineFT::Glyph *QFontEngineFT::loadGlyph(QGlyphSet *set, uint glyph, GlyphFormat format) const
{
//     Q_ASSERT(freetype->lock == 1);

    bool uploadToServer = false;
    if (format == Format_None) {
        if (defaultGlyphFormat != Format_None) {
            format = defaultGlyphFormat;
            if (canUploadGlyphsToServer)
                uploadToServer = true;
        } else {
            format = Format_Mono;
        }
    }

    Glyph *g = set->glyph_data.value(glyph);
    if (g && g->format == format) {
        if (uploadToServer && !g->uploadedToServer) {
            set->glyph_data[glyph] = 0;
            delete g;
            g = 0;
        } else {
            return g;
        }
    }

    QFontEngineFT::GlyphInfo info;

    Q_ASSERT(format != Format_None);
    bool hsubpixel = false;
    int vfactor = 1;
    int load_flags = FT_LOAD_DEFAULT | default_load_flags;
    if (outline_drawing) {
        load_flags = FT_LOAD_NO_BITMAP;
    } else if (format == Format_Mono) {
        load_flags |= FT_LOAD_TARGET_MONO;
    } else if (format == Format_A32) {
        if (subpixelType == QFontEngineFT::Subpixel_RGB || subpixelType == QFontEngineFT::Subpixel_BGR) {
            load_flags |= FT_LOAD_TARGET_LCD;
            hsubpixel = true;
        } else if (subpixelType == QFontEngineFT::Subpixel_VRGB || subpixelType == QFontEngineFT::Subpixel_VBGR) {
            load_flags |= FT_LOAD_TARGET_LCD_V;
            vfactor = 3;
        }

    }
#ifndef Q_WS_QWS
    if (format != Format_Mono)
        load_flags |= FT_LOAD_NO_BITMAP;
#endif

    bool transform = this->transform
                     || set->transformationMatrix.xx != 0x10000
                     || set->transformationMatrix.yy != 0x10000
                     || set->transformationMatrix.xy != 0
                     || set->transformationMatrix.yx != 0;

    if (transform)
        load_flags |= FT_LOAD_NO_BITMAP;

    FT_Face face = freetype->face;
    FT_Error err = FT_Load_Glyph(face, glyph, load_flags);
    if (err && (load_flags & FT_LOAD_NO_BITMAP)) {
        load_flags &= ~FT_LOAD_NO_BITMAP;
        err = FT_Load_Glyph(face, glyph, load_flags);
    }
    if (err == FT_Err_Too_Few_Arguments) {
        // this is an error in the bytecode interpreter, just try to run without it
        load_flags |= FT_LOAD_FORCE_AUTOHINT;
        err = FT_Load_Glyph(face, glyph, load_flags);
    }
    if (err != FT_Err_Ok)
        qWarning("load glyph failed err=%x face=%p, glyph=%d", err, face, glyph);

    if (outline_drawing) 
        return 0;
    
    FT_GlyphSlot slot = face->glyph;

    FT_Matrix matrix = freetype->matrix;

    int left  = slot->metrics.horiBearingX;
    int right = slot->metrics.horiBearingX + slot->metrics.width;
    int top    = slot->metrics.horiBearingY;
    int bottom = slot->metrics.horiBearingY - slot->metrics.height;
    if(transform && slot->format != FT_GLYPH_FORMAT_BITMAP) {
        int l, r, t, b;
        FT_Vector vector;
        vector.x = left;
        vector.y = top;
        FT_Vector_Transform(&vector, &matrix);
        l = r = vector.x;
        t = b = vector.y;
        vector.x = right;
        vector.y = top;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        vector.x = right;
        vector.y = bottom;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        vector.x = left;
        vector.y = bottom;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        left = l;
        right = r;
        top = t;
        bottom = b;
    }
    left = FLOOR(left);
    right = CEIL(right);
    bottom = FLOOR(bottom);
    top = CEIL(top);

    int hpixels = TRUNC(right - left);
    if (hsubpixel)
        hpixels = hpixels*3 + 8;
    info.width = hpixels;
    info.height = TRUNC(top - bottom);
    info.x = -TRUNC(left);
    info.y = TRUNC(top);
    info.xOff = TRUNC(ROUND(slot->advance.x));
    info.yOff = 0;
    if (hsubpixel) {
        info.width /= 3;
        info.x += 1;
    }

    int pitch = (format == Format_Mono ? ((info.width + 31) & ~31) >> 3 :
                 (format == Format_A8 ? (info.width + 3) & ~3 : info.width * 4));
    int size = pitch * info.height;
    uchar *glyph_buffer = new uchar[size];

    if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_Bitmap bitmap;
        bitmap.rows = info.height*vfactor;
        bitmap.width = hpixels;
        bitmap.pitch = format == Format_Mono ? (((info.width + 31) & ~31) >> 3) : ((bitmap.width + 3) & ~3);
        if (!hsubpixel && vfactor == 1)
            bitmap.buffer = glyph_buffer;
        else
            bitmap.buffer = new uchar[bitmap.rows*bitmap.pitch];
        memset(bitmap.buffer, 0, bitmap.rows*bitmap.pitch);
        bitmap.pixel_mode = format == Format_Mono ? ft_pixel_mode_mono : ft_pixel_mode_grays;
        FT_Matrix matrix;
        matrix.xx = (hsubpixel ? 3 : 1) << 16;
        matrix.yy = vfactor << 16;
        matrix.yx = matrix.xy = 0;

        FT_Outline_Transform(&slot->outline, &matrix);
        FT_Outline_Translate (&slot->outline, (hsubpixel ? -3*left +(4<<6) : -left), -bottom*vfactor);
        FT_Outline_Get_Bitmap(library, &slot->outline, &bitmap);
        if (hsubpixel) {
            Q_ASSERT (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
            Q_ASSERT(antialias);
            const uchar *src = bitmap.buffer;
            uchar *convoluted = new uchar[bitmap.rows*bitmap.pitch];
            uchar *c = convoluted;
            // convolute the bitmap with a triangle filter to get rid of color fringes
            // If we take account for a gamma value of 2, we end up with
            // weights of 1, 4, 9, 4, 1. We use an approximation of 1, 3, 8, 3, 1 here,
            // as this nicely sums up to 16 :)
            int h = info.height;
            while (h--) {
                c[0] = c[1] = 0;
                //
                for (int x = 2; x < bitmap.width - 2; ++x) {
                    uint sum = src[x-2] + 3*src[x-1] + 8*src[x] + 3*src[x+1] + src[x+2];
                    c[x] = (uchar) (sum >> 4);
                }
                c[bitmap.width - 2] = c[bitmap.width -1] = 0;
                src += bitmap.pitch;
                c += bitmap.pitch;
            }

            uint *dst = (uint *)glyph_buffer;
            src = convoluted;
            h = info.height;
            if (subpixelType == QFontEngineFT::Subpixel_RGB) {
                while (h--) {
                    uint *dd = dst;
                    for (int x = 1; x < bitmap.width - 1; x += 3) {
                        uint red = src[x];
                        uint green = src[x+1];
                        uint blue = src[x+2];
                        uint res = (red << 16) + (green << 8) + blue;
                        *dd = res;
                        ++dd;
                    }
                    dst += info.width;
                    src += bitmap.pitch;
                }
            } else {
                while (h--) {
                    uint *dd = dst;
                    for (int x = 1; x < bitmap.width - 1; x += 3) {
                        uint blue = src[x];
                        uint green = src[x+1];
                        uint red = src[x+2];
                        uint res = (red << 16) + (green << 8) + blue;
                        *dd = res;
                        ++dd;
                    }
                    dst += info.width;
                    src += bitmap.pitch;
                }
            }
            delete [] convoluted;
            delete [] bitmap.buffer;
        } else if (vfactor != 1) {
            uchar *src = bitmap.buffer;
            size = info.width * 4 * info.height;
            uint *dst = (uint *)glyph_buffer;
            int h = info.height;
            if (subpixelType == QFontEngineFT::Subpixel_VRGB) {
                while (h--) {
                    for (int x = 0; x < info.width; x++) {
                        uint red = src[x];
                        uint green = src[x+bitmap.pitch];
                        uint blue = src[x+2*bitmap.pitch];
                        uint high = (red*subpixel_filter[0][0] + green*subpixel_filter[0][1] + blue*subpixel_filter[0][2]) >> 8;
                        uint mid = (red*subpixel_filter[1][0] + green*subpixel_filter[1][1] + blue*subpixel_filter[1][2]) >> 8;
                        uint low = (red*subpixel_filter[2][0] + green*subpixel_filter[2][1] + blue*subpixel_filter[2][2]) >> 8;
                        uint res = (high << 16) + (mid << 8) + low;
                        dst[x] = res;
                    }
                    dst += info.width;
                    src += 3*bitmap.pitch;
                }
            } else {
                while (h--) {
                    for (int x = 0; x < info.width; x++) {
                        uint blue = src[x];
                        uint green = src[x+bitmap.pitch];
                        uint red = src[x+2*bitmap.pitch];
                        uint high = (red*subpixel_filter[0][0] + green*subpixel_filter[0][1] + blue*subpixel_filter[0][2]) >> 8;
                        uint mid = (red*subpixel_filter[1][0] + green*subpixel_filter[1][1] + blue*subpixel_filter[1][2]) >> 8;
                        uint low = (red*subpixel_filter[2][0] + green*subpixel_filter[2][1] + blue*subpixel_filter[2][2]) >> 8;
                        uint res = (high << 16) + (mid << 8) + low;
                        dst[x] = res;
                    }
                    dst += info.width;
                    src += 3*bitmap.pitch;
                }
            }
            delete [] bitmap.buffer;
        }
    } else if (slot->format == FT_GLYPH_FORMAT_BITMAP) {
        Q_ASSERT(slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);
        uchar *src = slot->bitmap.buffer;
        uchar *dst = glyph_buffer;
        int h = slot->bitmap.rows;
        if (format == Format_Mono) {
            int bytes = ((info.width + 7) & ~7) >> 3;
            while (h--) {
                memcpy (dst, src, bytes);
                dst += pitch;
                src += slot->bitmap.pitch;
            }
        } else {
            if (hsubpixel) {
                while (h--) {
                    uint *dd = (uint *)dst;
                    *dd++ = 0;
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        uint a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xffffff : 0x000000);
                        *dd++ = a;
                    }
                    *dd++ = 0;
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            } else if (vfactor != 1) {
                while (h--) {
                    uint *dd = (uint *)dst;
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        uint a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xffffff : 0x000000);
                        *dd++ = a;
                    }
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            } else {
                while (h--) {
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        unsigned char a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xff : 0x00);
                        dst[x] = a;
                    }
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            }
        }
    } else {
        qWarning("QFontEngine: Glyph neither outline nor bitmap format=%d", slot->format);
        delete [] glyph_buffer;
        return 0;
    }

    bool large_glyph = (((signed char)(slot->linearHoriAdvance>>16) != slot->linearHoriAdvance>>16)
                        || ((uchar)(info.width) != info.width)
                        || ((uchar)(info.height) != info.height)
                        || ((signed char)(info.x) != info.x)
                        || ((signed char)(info.y) != info.y)
                        || ((signed char)(info.xOff) != info.xOff));

    if (large_glyph) {
//         qDebug("got a large glyph!");
        delete [] glyph_buffer;
        return 0;
    }

    if (!g) {
        g = new Glyph;
        g->uploadedToServer = false;
        g->data = 0;
    }

    g->linearAdvance = slot->linearHoriAdvance >> 10;
    g->width = info.width;
    g->height = TRUNC(top - bottom);
    g->x = -info.x;
    g->y = TRUNC(top);
    g->advance = TRUNC(ROUND(slot->advance.x));
    g->format = format;
    delete [] g->data;
    g->data = glyph_buffer;

    if (uploadToServer) {
        uploadGlyphToServer(set, glyph, g, &info, size);
    }

    set->glyph_data[glyph] = g;

    return g;
}

bool QFontEngineFT::uploadGlyphToServer(QGlyphSet *set, uint glyphid, Glyph *g, GlyphInfo *info, int glyphDataSize) const
{
    Q_UNUSED(set);
    Q_UNUSED(glyphid);
    Q_UNUSED(g);
    Q_UNUSED(info);
    Q_UNUSED(glyphDataSize);
    return false;
}

QFontEngine::FaceId QFontEngineFT::faceId() const
{
    return face_id;
}

QFontEngine::Properties QFontEngineFT::properties() const
{
    Properties p = freetype->properties();
    if (p.postscriptName.isEmpty()) {
        p.postscriptName = fontDef.family.toUtf8();
#ifndef QT_NO_PRINTER
        p.postscriptName = QPdf::stripSpecialCharacters(p.postscriptName);
#endif
    }

    return freetype->properties();
}

QByteArray QFontEngineFT::getSfntTable(uint tag) const
{
    return freetype->getSfntTable(tag);
}

int QFontEngineFT::synthesized() const
{
    int s = 0;
    if ((fontDef.style != QFont::StyleNormal) && !(freetype->face->style_flags & FT_STYLE_FLAG_ITALIC))
        s = SynthesizedItalic;
    if (fontDef.stretch != 100 && FT_IS_SCALABLE(freetype->face))
        s |= SynthesizedStretch;
    return s;
}

QFixed QFontEngineFT::ascent() const
{
    return QFixed::fromFixed(metrics.ascender);
}

QFixed QFontEngineFT::descent() const
{
    return QFixed::fromFixed(-metrics.descender + (1<<6));
}

QFixed QFontEngineFT::leading() const
{
    return QFixed::fromFixed(metrics.height - metrics.ascender + metrics.descender);
}

QFixed QFontEngineFT::xHeight() const
{
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(freetype->face, ft_sfnt_os2);
    if (os2 && os2->sxHeight)
        return QFixed(os2->sxHeight*freetype->face->size->metrics.y_ppem)/freetype->face->units_per_EM;
    return QFontEngine::xHeight();
}

QFixed QFontEngineFT::averageCharWidth() const
{
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(freetype->face, ft_sfnt_os2);
    if (os2 && os2->xAvgCharWidth)
        return QFixed(os2->xAvgCharWidth*freetype->face->size->metrics.y_ppem)/freetype->face->units_per_EM;
    return QFontEngine::averageCharWidth();
}

qreal QFontEngineFT::maxCharWidth() const
{
    return metrics.max_advance >> 6;
}

static const ushort char_table[] = {
        40,
        67,
        70,
        75,
        86,
        88,
        89,
        91,
        102,
        114,
        124,
        127,
        205,
        645,
        884,
        922,
        1070,
        12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


qreal QFontEngineFT::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        (void) minRightBearing(); // calculates both
    return lbearing.toReal();
}

qreal QFontEngineFT::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        lbearing = rbearing = 0;
        const QChar *ch = (const QChar *)(const void*)char_table;
        QGlyphLayout glyphs[char_table_entries];
        int ng = char_table_entries;
        stringToCMap(ch, char_table_entries, glyphs, &ng, 0);
        while (--ng) {
            if (glyphs[ng].glyph) {
                glyph_metrics_t gi = const_cast<QFontEngineFT *>(this)->boundingBox(glyphs[ng].glyph);
                lbearing = qMin(lbearing, gi.x);
                rbearing = qMin(rbearing, (gi.xoff - gi.x - gi.width));
            }
        }
    }
    return rbearing.toReal();
}

QFixed QFontEngineFT::lineThickness() const
{
    return line_thickness;
}

QFixed QFontEngineFT::underlinePosition() const
{
    return underline_position;
}

void QFontEngineFT::doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const
{
    if (!kerning_pairs_loaded) {
        kerning_pairs_loaded = true;
        if (freetype->face->size->metrics.x_ppem != 0) {
            lockFace();
            QFixed scalingFactor(freetype->face->units_per_EM/freetype->face->size->metrics.x_ppem);
            unlockFace();
            const_cast<QFontEngineFT *>(this)->loadKerningPairs(scalingFactor);
        }
    }
    QFontEngine::doKerning(num_glyphs, g, flags);
}

QOpenType *QFontEngineFT::openType() const
{
#ifndef QT_NO_OPENTYPE
    if (_openType)
         return _openType;

    FT_Face face = lockFace();
    if (!face || !FT_IS_SFNT(face)) {
        unlockFace();
        return 0;
    }

    _openType = new QOpenType(const_cast<QFontEngineFT *>(this), face);
    unlockFace();
#endif
    return _openType;
}

QFontEngineFT::QGlyphSet *QFontEngineFT::loadTransformedGlyphSet(glyph_t *glyphs, int num_glyphs, const QTransform &matrix,
                                                                 GlyphFormat format)
{
    // don't try to load huge fonts
    if (fontDef.pixelSize * qSqrt(matrix.det()) >= 64)
        return 0;

    FT_Matrix m;
    m.xx = FT_Fixed(matrix.m11() * 65536);
    m.xy = FT_Fixed(-matrix.m21() * 65536);
    m.yx = FT_Fixed(-matrix.m12() * 65536);
    m.yy = FT_Fixed(matrix.m22() * 65536);

    QGlyphSet *gs = 0;

    for (int i = 0; i < transformedGlyphSets.count(); ++i) {
        const QGlyphSet &g = transformedGlyphSets.at(i);
        if (g.transformationMatrix.xx == m.xx
            && g.transformationMatrix.xy == m.xy
            && g.transformationMatrix.yx == m.yx
            && g.transformationMatrix.yy == m.yy) {

            // found a match, move it to the front
            transformedGlyphSets.move(i, 0);
            gs = &transformedGlyphSets[0];
            break;
        }
    }

    if (!gs) {
        // don't cache more than 10 transformations
        if (transformedGlyphSets.count() >= 10) {
            transformedGlyphSets.move(transformedGlyphSets.size() - 1, 0);
            freeServerGlyphSet(transformedGlyphSets.at(0).id);
        } else {
            transformedGlyphSets.prepend(QGlyphSet());
        }
        gs = &transformedGlyphSets[0];

        qDeleteAll(gs->glyph_data);
        gs->glyph_data.clear();

        gs->id = allocateServerGlyphSet();

        gs->transformationMatrix = m;
    }

    FT_Face face = 0;
    bool lockedFace = false;

    for (int i = 0; i < num_glyphs; ++i) {
        if (!gs->glyph_data.contains(glyphs[i])) {
            if (!lockedFace) {
                face = lockFace();
                m = this->matrix;
                FT_Matrix_Multiply(&gs->transformationMatrix, &m);
                FT_Set_Transform(face, &m, 0);
                freetype->matrix = m;
                lockedFace = true;
            }
            if (!loadGlyph(gs, glyphs[i], format)) {
                FT_Set_Transform(face, &freetype->matrix, 0);
                unlockFace();
                return 0;
            }
        }
    }

    if (lockedFace) {
        FT_Set_Transform(face, &freetype->matrix, 0);
        unlockFace();
    }

    return gs;
}

void QFontEngineFT::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    FT_Face face = lockFace(Unscaled);
    FT_Set_Transform(face, 0, 0);
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
    unlockFace();
}

static inline unsigned int getChar(const QChar *str, int &i, const int len)
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

bool QFontEngineFT::canRender(const QChar *string, int len)
{
    FT_Face face = freetype->face;
#if 0
    if (_cmap != -1) {
        lockFace();
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FcCharSetHasChar (_font->charset, uc) && getAdobeCharIndex(face, _cmap, uc) == 0) {
                allExist = false;
                break;
            }
        }
        unlockFace();
    } else
#endif
    {
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FT_Get_Char_Index(face, uc))
                    return false;
        }
    }
    return true;
}

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (FT_IS_SCALABLE(freetype->face)) {
        QFontEngine::addOutlineToPath(x, y, glyphs, numGlyphs, path, flags);
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
    }
}

void QFontEngineFT::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                                    QPainterPath *path, QTextItem::RenderFlags)
{
    FT_Face face = lockFace(Unscaled);

    for (int gl = 0; gl < numGlyphs; gl++) {
        FT_UInt glyph = glyphs[gl];

        FT_Load_Glyph(face, glyph, FT_LOAD_NO_BITMAP);

        FT_GlyphSlot g = face->glyph;
        if (g->format != FT_GLYPH_FORMAT_OUTLINE)
            continue;
        QFreetypeFace::addGlyphToPath(face, g, positions[gl], path, xsize, ysize);
    }
    unlockFace();
}

bool QFontEngineFT::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                 QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    int glyph_pos = 0;
    if (freetype->symbol_map) {
        FT_Face face = freetype->face;
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = getChar(str, i, len);
            if (mirrored)
                uc = QChar::mirroredChar(uc);
            glyphs[glyph_pos].glyph = uc < QFreetypeFace::cmapCacheSize ? freetype->cmapCache[uc] : 0;
            if ( !glyphs[glyph_pos].glyph ) {
                glyph_t glyph;
#if !defined(QT_NO_FONTCONFIG)
                if (FcCharSetHasChar(freetype->charset, uc)) {
#else
                if (false) {
#endif
                redo0:
                    glyph = FT_Get_Char_Index(face, uc);
                    if (!glyph && (uc == 0xa0 || uc == 0x9)) {
                        uc = 0x20;
                        goto redo0;
                    }
                } else {
                    FT_Set_Charmap(face, freetype->symbol_map);
                    glyph = FT_Get_Char_Index(face, uc);
                    FT_Set_Charmap(face, freetype->unicode_map);
                }
                glyphs[glyph_pos].glyph = glyph;
                if (uc < QFreetypeFace::cmapCacheSize)
                    freetype->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    } else {
        FT_Face face = freetype->face;
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            if (mirrored)
                uc = QChar::mirroredChar(uc);
            glyphs[glyph_pos].glyph = uc < QFreetypeFace::cmapCacheSize ? freetype->cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph
#if !defined(QT_NO_FONTCONFIG)
                && FcCharSetHasChar(freetype->charset, uc)
#endif
                ) {
            redo:
                glyph_t glyph = FT_Get_Char_Index(face, uc);
                if (!glyph && (uc == 0xa0 || uc == 0x9)) {
                    uc = 0x20;
                    goto redo;
                }
                glyphs[glyph_pos].glyph = glyph;
                if (uc < QFreetypeFace::cmapCacheSize)
                    freetype->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;

    if (flags & QTextEngine::GlyphIndicesOnly)
        return true;

    recalcAdvances(*nglyphs, glyphs, flags);

    return true;
}

void QFontEngineFT::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    FT_Face face = 0;
    if (flags & QTextEngine::DesignMetrics) {
        for (int i = 0; i < len; i++) {
            Glyph *g = defaultGlyphSet.glyph_data.value(glyphs[i].glyph);
            if (g) {
                glyphs[i].advance.x = QFixed::fromFixed(g->linearAdvance);
            } else {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
                glyphs[i].advance.x = QFixed::fromFixed(face->glyph->linearHoriAdvance >> 10);
            }
            glyphs[i].advance.y = 0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            Glyph *g = defaultGlyphSet.glyph_data.value(glyphs[i].glyph);
            if (g) {
                glyphs[i].advance.x = QFixed(g->advance);
            } else {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
                glyphs[i].advance.x = QFixed::fromFixed(face->glyph->metrics.horiAdvance);
            }
            glyphs[i].advance.y = 0;
        }
    }
    if (face)
        unlockFace();
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{

    FT_Face face = 0;

    glyph_metrics_t overall;
    // initialize with line height, we get the same behaviour on all platforms
    overall.y = -ascent();
    overall.height = ascent() + descent() + 1;

    QFixed ymax = 0;
    QFixed xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        Glyph *g = defaultGlyphSet.glyph_data.value(glyphs[i].glyph);
        if (!g) {
            if (!face)
                face = lockFace();
            g = loadGlyph(glyphs[i].glyph);
        }
        if (g) {
            QFixed x = overall.xoff + glyphs[i].offset.x + g->x;
            QFixed y = overall.yoff + glyphs[i].offset.y - g->y;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + g->width);
            ymax = qMax(ymax, y + g->height);
            overall.xoff += qRound(g->advance);
        } else {
            int left  = FLOOR(face->glyph->metrics.horiBearingX);
            int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
            int top    = CEIL(face->glyph->metrics.horiBearingY);
            int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

            QFixed x = overall.xoff + glyphs[i].offset.x - (-TRUNC(left));
            QFixed y = overall.yoff + glyphs[i].offset.y - TRUNC(top);
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + TRUNC(right - left));
            ymax = qMax(ymax, y + TRUNC(top - bottom));
            overall.xoff += qRound(TRUNC(ROUND(face->glyph->advance.x)));
        }
    }
    overall.height = qMax(overall.height, ymax - overall.y);
    overall.width = xmax - overall.x;

    if (face)
        unlockFace();

    return overall;
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    FT_Face face = 0;
    glyph_metrics_t overall;
    Glyph *g = defaultGlyphSet.glyph_data.value(glyph);
    if (!g) {
        face = lockFace();
        g = loadGlyph(glyph);
    }
    if (g) {
        overall.x = g->x;
        overall.y = -g->y;
        overall.width = g->width;
        overall.height = g->height;
        overall.xoff = g->advance;
    } else {
        int left  = FLOOR(face->glyph->metrics.horiBearingX);
        int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
        int top    = CEIL(face->glyph->metrics.horiBearingY);
        int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

        overall.width = TRUNC(right-left);
        overall.height = TRUNC(top-bottom);
        overall.x = TRUNC(left);
        overall.y = -TRUNC(top);
        overall.xoff = TRUNC(ROUND(face->glyph->advance.x));
    }
    if (face)
        unlockFace();
    return overall;
}

QImage QFontEngineFT::alphaMapForGlyph(glyph_t g)
{
    lockFace();

    Glyph *glyph = loadGlyph(g, Format_A8);
    if (!glyph) {
        unlockFace();
        return QFontEngine::alphaMapForGlyph(g);
    }

    Q_ASSERT(glyph->format == QFontEngineFT::Format_A8);

    const int pitch = (glyph->width + 3) & ~3;

    QImage img(glyph->width, glyph->height, QImage::Format_Indexed8);
    QVector<QRgb> colors(256);
    for (int i=0; i<256; ++i)
        colors[i] = qRgba(0, 0, 0, i);
    img.setColorTable(colors);

    for (int y = 0; y < glyph->height; ++y)
        memcpy(img.scanLine(y), &glyph->data[y * pitch], pitch);
    unlockFace();

    return img;
}

void QFontEngineFT::removeGlyphFromCache(glyph_t glyph)
{
    delete defaultGlyphSet.glyph_data.take(glyph);
}

int QFontEngineFT::glyphCount() const
{
    int count = 0;
    FT_Face face = lockFace();
    if (face) {
        count = face->num_glyphs;
        unlockFace();
    }
    return count;
}

FT_Face QFontEngineFT::lockFace(Scaling scale) const
{
    freetype->lock();
    FT_Face face = freetype->face;
    if (scale == Unscaled) {
        FT_Set_Char_Size(face, face->units_per_EM << 6, face->units_per_EM << 6, 0, 0);
        freetype->xsize = face->units_per_EM << 6;
        freetype->ysize = face->units_per_EM << 6;
    } else if (freetype->xsize != xsize || freetype->ysize != ysize) {
        FT_Set_Char_Size(face, xsize, ysize, 0, 0);
        freetype->xsize = xsize;
        freetype->ysize = ysize;
    }
    if (freetype->matrix.xx != matrix.xx ||
        freetype->matrix.yy != matrix.yy ||
        freetype->matrix.xy != matrix.xy ||
        freetype->matrix.yx != matrix.yx) {
        freetype->matrix = matrix;
        FT_Set_Transform(face, &freetype->matrix, 0);
    }

    return face;
}

void QFontEngineFT::unlockFace() const
{
    freetype->unlock();
}

FT_Face QFontEngineFT::non_locked_face() const
{
    return freetype->face;
}


QFontEngineFT::QGlyphSet::QGlyphSet()
    : id(0)
{
    transformationMatrix.xx = 0x10000;
    transformationMatrix.yy = 0x10000;
    transformationMatrix.xy = 0;
    transformationMatrix.yx = 0;
}

QFontEngineFT::QGlyphSet::~QGlyphSet()
{
    qDeleteAll(glyph_data);
}

unsigned long QFontEngineFT::allocateServerGlyphSet()
{
    return 0;
}

void QFontEngineFT::freeServerGlyphSet(unsigned long id)
{
    Q_UNUSED(id);
}

#endif // QT_NO_FREETYPE
