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

#ifndef QFONTENGINE_WIN_P_H
#define QFONTENGINE_WIN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);
    ~QFontEngineWin();

    virtual Properties properties() const;
    virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    virtual FaceId faceId() const;
    QByteArray getSfntTable(uint tag) const;
    virtual int synthesized() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);
    void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                         QPainterPath *path, QTextItem::RenderFlags flags);

    HGDIOBJ selectDesignFont(QFixed *) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;
    QFixed averageCharWidth() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    const char *name() const;

    bool canRender(const QChar *string, int len);

    Type type() const;

    virtual QImage alphaMapForGlyph(glyph_t);

    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
    mutable QFixed *designAdvances;
    mutable int designAdvancesSize;
};

class QFontEngineMultiWin : public QFontEngineMulti
{
public:
    QFontEngineMultiWin(QFontEngineWin *first, const QStringList &fallbacks);
    void loadEngine(int at);

    QStringList fallbacks;
};

#endif // QFONTENGINE_WIN_P_H
