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

#ifndef QABSTRACTFONTENGINE_QWS_H
#define QABSTRACTFONTENGINE_QWS_H

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qfactoryinterface.h>
#include <QtGui/qpaintengine.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QFontEngineInfoPrivate;

class Q_GUI_EXPORT QFontEngineInfo
{
public:
    QFontEngineInfo();
    explicit QFontEngineInfo(const QString &family);
    QFontEngineInfo(const QFontEngineInfo &other);
    QFontEngineInfo &operator=(const QFontEngineInfo &other);
    ~QFontEngineInfo();

    void setFamily(const QString &name);
    QString family() const;

    void setPixelSize(qreal size);
    qreal pixelSize() const;

    void setWeight(int weight);
    int weight() const;

    void setStyle(QFont::Style style);
    QFont::Style style() const;

private:
    QFontEngineInfoPrivate *d;
};

class QAbstractFontEngine;

struct Q_GUI_EXPORT QFontEngineFactoryInterface : public QFactoryInterface
{
     virtual QAbstractFontEngine *create(const QFontEngineInfo &info) = 0;
     virtual QList<QFontEngineInfo> availableFontEngines() const = 0;
};

#define QFontEngineFactoryInterface_iid "com.trolltech.Qt.QFontEngineFactoryInterface"
Q_DECLARE_INTERFACE(QFontEngineFactoryInterface, QFontEngineFactoryInterface_iid)

class QFontEnginePluginPrivate;

class Q_GUI_EXPORT QFontEnginePlugin : public QObject, public QFontEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QFontEngineFactoryInterface:QFactoryInterface)
public:
    QFontEnginePlugin(const QString &foundry, QObject *parent = 0);
    ~QFontEnginePlugin();

    virtual QStringList keys() const;

    virtual QAbstractFontEngine *create(const QFontEngineInfo &info) = 0;
    virtual QList<QFontEngineInfo> availableFontEngines() const = 0;

private:
    Q_DECLARE_PRIVATE(QFontEnginePlugin)
    Q_DISABLE_COPY(QFontEnginePlugin)
};

class QAbstractFontEnginePrivate;

class Q_GUI_EXPORT QAbstractFontEngine : public QObject
{
    Q_OBJECT
public:
    enum Capability {
        RenderGlyphs  = 1,
        OutlineGlyphs = 2
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    explicit QAbstractFontEngine(Capabilities capabilities, QObject *parent = 0);
    ~QAbstractFontEngine();

    typedef int Fixed; // 26.6

    struct FixedPoint
    {
        Fixed x, y;
    };

    struct GlyphMetrics
    {
        inline GlyphMetrics()
            : x(0), y(0), width(0), height(0),
              advance(0) {}
        Fixed x, y;
        Fixed width, height;
        Fixed advance;
    };

    enum FontProperty {
        Ascent,
        Descent,
        Leading,
        XHeight,
        AverageCharWidth,
        LineThickness,
        UnderlinePosition,
        MaxCharWidth,
        MinLeftBearing,
        MinRightBearing,
        GlyphCount,

        // hints
        ShareGlyphsHint,
        OutlineGlyphsHint
    };

    // keep in sync with QTextEngine::ShaperFlag!!
    enum TextShapingFlag {
        RightToLeft      = 0x0001,
        UseDesignMetrics = 0x0002
    };
    Q_DECLARE_FLAGS(TextShapingFlags, TextShapingFlag)

    enum Extension {
        GetTrueTypeTable
    };

    Capabilities capabilities() const;

    virtual bool convertStringToGlyphIndices(const QChar *string, int length, uint *glyphs, int *numGlyphs, TextShapingFlags flags) const = 0;

    // Fixed or FixedPoint?
    virtual void getGlyphAdvances(const uint *glyphs, int numGlyphs, Fixed *advances, TextShapingFlags flags) const = 0;

    virtual GlyphMetrics glyphMetrics(uint glyph) const = 0;

    virtual QVariant fontProperty(FontProperty property) const = 0;

    virtual QImage renderGlyph(uint glyph);

    virtual void addGlyphOutlinesToPath(uint *glyphs, int numGlyphs, FixedPoint *positions, QPainterPath *path, QTextItem::RenderFlags flags);

    virtual bool supportsExtension(Extension extension) const; // ?
    virtual QVariant extension(Extension extension, const QVariant &argument = QVariant());

private:
    Q_DECLARE_PRIVATE(QAbstractFontEngine)
    Q_DISABLE_COPY(QAbstractFontEngine)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractFontEngine::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractFontEngine::TextShapingFlags)

QT_END_HEADER

#endif
