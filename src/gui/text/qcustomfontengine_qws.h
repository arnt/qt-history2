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

#ifndef QCUSTOMFONTENGINE_QWS_H
#define QCUSTOMFONTENGINE_QWS_H

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
};

class QCustomFontEngine;

class Q_GUI_EXPORT QCustomFontEngineFactory : public QObject
{
    Q_OBJECT
public:
    QCustomFontEngineFactory(QObject *parent = 0);

    virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
};

struct Q_GUI_EXPORT QCustomFontEngineFactoryInterface : public QFactoryInterface
{
     virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
     virtual QList<QCustomFontInfo> availableFonts() const = 0;
};

#define QCustomFontEngineFactoryInterface_iid "com.trolltech.Qt.QCustomFontEngineFactoryInterface"
Q_DECLARE_INTERFACE(QCustomFontEngineFactoryInterface, QCustomFontEngineFactoryInterface_iid)

class QCustomFontEnginePluginPrivate;

class Q_GUI_EXPORT QCustomFontEnginePlugin : public QObject, public QCustomFontEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QCustomFontEngineFactoryInterface:QFactoryInterface)
public:
    QCustomFontEnginePlugin(const QString &foundry, QObject *parent = 0);
    ~QCustomFontEnginePlugin();

    virtual QStringList keys() const;

    virtual QCustomFontEngine *create(const QCustomFontInfo &info) = 0;
    virtual QList<QCustomFontInfo> availableFonts() const = 0;

private:
    Q_DECLARE_PRIVATE(QCustomFontEnginePlugin)
    Q_DISABLE_COPY(QCustomFontEnginePlugin)
};

class QCustomFontEnginePrivate;

class Q_GUI_EXPORT QCustomFontEngine : public QObject
{
    Q_OBJECT
public:
    enum FontEngineFeature {
        GlyphRendering = 1,
        GlyphOutlines  = 2
    };
    Q_DECLARE_FLAGS(FontEngineFeatures, FontEngineFeature)

    explicit QCustomFontEngine(FontEngineFeatures supportedFeatures, QObject *parent = 0);
    ~QCustomFontEngine();

    typedef int Fixed; // 26.6

    struct GlyphMetrics
    {
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
        TotalGlyphCount,

        // hints
        GlyphShareHint,
        OutlineRenderHint
    };

    enum Extension {
        TrueTypeFontTable,
    };

    FontEngineFeatures supportedFeatures() const;

    virtual bool stringToGlyphIndices(const QChar *string, int length, uint *glyphs, int *numGlyphs, uint flags) const = 0;
    virtual void getGlyphAdvances(const uint *glyphs, int numGlyphs, Fixed *advances, uint flags) const = 0;
    virtual GlyphMetrics getGlyphMetrics(uint glyph) const = 0;
    virtual QVariant fontProperty(FontProperty property) const = 0;

    virtual QImage renderGlyph(uint glyph);
    virtual void addGlyphsToPath(uint *glyphs, int numGlyphs, Fixed *positions, QPainterPath *path, QTextItem::RenderFlags flags);

    virtual QVariant extension(Extension extension, const QVariant &argument = QVariant());

private:
    Q_DECLARE_PRIVATE(QCustomFontEngine)
    Q_DISABLE_COPY(QCustomFontEngine)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCustomFontEngine::FontEngineFeatures)

QT_END_HEADER

#endif
