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

class Q_GUI_EXPORT QCustomFontInfo : public QHash<int, QVariant>
{
public:
    enum Property {
        Family,
        PixelSize,
        Weight,
        Style,
        NoAntialiasing,
        SupportedWritingSystems
    };

    QCustomFontInfo();
    explicit QCustomFontInfo(const QString &family);
    QCustomFontInfo(const QCustomFontInfo &other);
    QCustomFontInfo &operator=(const QCustomFontInfo &other);

    inline void setFamily(const QString &name)
    { insert(Family, name); }
    inline QString family() const
    { return value(Family).toString(); }

    void setPixelSize(qreal size);
    qreal pixelSize() const;

    inline void setWeight(int weight)
    { insert(Weight, weight); }
    inline int weight() const
    { return value(Weight).toInt(); }

    inline void setStyle(int style)
    { insert(Style, style); }
    inline int style() const
    { return value(Style).toInt(); }

    // ### missing foundry
};

class QAbstractFontEngine;

struct Q_GUI_EXPORT QFontEngineFactoryInterface : public QFactoryInterface
{
     virtual QAbstractFontEngine *create(const QCustomFontInfo &info) = 0;
     virtual QList<QCustomFontInfo> availableFonts() const = 0;
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

    virtual QAbstractFontEngine *create(const QCustomFontInfo &info) = 0;
    virtual QList<QCustomFontInfo> availableFonts() const = 0;

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
        GlyphRendering = 1,
        GlyphOutlines  = 2
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
        GlyphShareHint,
        OutlineRenderHint
    };

    enum Extension {
        GetTrueTypeTable
    };

    Capabilities capabilities() const;

    virtual bool convertStringToGlyphIndices(const QChar *string, int length, uint *glyphs, int *numGlyphs, uint flags) const = 0;

    // Fixed or FixedPoint?
    virtual void getGlyphAdvances(const uint *glyphs, int numGlyphs, Fixed *advances, uint flags) const = 0;

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

QT_END_HEADER

#endif
