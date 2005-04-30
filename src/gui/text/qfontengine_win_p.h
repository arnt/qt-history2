#ifndef QFONTENGINE_WIN_P_H
#define QFONTENGINE_WIN_P_H

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);
    ~QFontEngineWin();

    FECaps capabilites() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;

    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);
    virtual void doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const;

    HGDIOBJ selectDesignFont(float *) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    qreal ascent() const;
    qreal descent() const;
    qreal leading() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    const char *name() const;

    bool canRender(const QChar *string, int len);

    Type type() const;

    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
    mutable float *designAdvances;
    mutable int designAdvancesSize;
};

class QFontEngineMultiWin : public QFontEngineMulti
{
public:
    QFontEngineMultiWin(QFontEngineWin *first, const QStringList &fallbacks);
    void loadEngine(int at);

    QStringList fallbacks;
};

#endif
