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

#ifndef QABSTRACTFONTENGINE_P_H
#define QABSTRACTFONTENGINE_P_H

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

#include "qfontengine_p.h"
#include "qabstractfontengine_qws.h"

QT_BEGIN_NAMESPACE

class QCustomFontEngine;

class QProxyFontEngine : public QFontEngine
{
    Q_OBJECT
public:
    QProxyFontEngine(QAbstractFontEngine *engine, const QFontDef &def);
    virtual ~QProxyFontEngine();

    virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    virtual QImage alphaMapForGlyph(glyph_t);
    virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags);
    virtual glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    virtual glyph_metrics_t boundingBox(glyph_t glyph);

    virtual QFixed ascent() const;
    virtual QFixed descent() const;
    virtual QFixed leading() const;
    virtual QFixed xHeight() const;
    virtual QFixed averageCharWidth() const;
    virtual QFixed lineThickness() const;
    virtual QFixed underlinePosition() const;
    virtual qreal maxCharWidth() const;
    virtual qreal minLeftBearing() const;
    virtual qreal minRightBearing() const;
    virtual int glyphCount() const;

    virtual bool canRender(const QChar *string, int len);

    virtual Type type() const { return Proxy; }
    virtual const char *name() const { return "proxy engine"; }

#if !defined(Q_WS_X11) && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
    virtual void draw(QPaintEngine *, qreal, qreal, const QTextItemInt &);
#endif

    inline QAbstractFontEngine::Capabilities capabilities() const
    { return engineCapabilities; }

    bool drawAsOutline() const;

private:
    QAbstractFontEngine *engine;
    QAbstractFontEngine::Capabilities engineCapabilities;
};

QT_END_NAMESPACE

#endif
