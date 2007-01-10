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

#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include "domimage.h"

#include <QPainter>
#include <QString>
#include <QStack>
#include <QMetaType>
#include <QExplicitlySharedDataPointer>

#include <qscriptengine.h>

class CanvasGradientData;

typedef QExplicitlySharedDataPointer<CanvasGradientData> CanvasGradient;

class CanvasGradientData: public QSharedData
{
public:
    CanvasGradientData(const QGradient &g)
        : gradient(g) {}

    const QGradient &qgradient() const
    {
        return gradient;
    }

    static void setup(QScriptEngine *e);

private: // API
    static QScriptValue addColorStop(QScriptContext *, QScriptEngine *);

public: // attributes
    QGradient gradient;
};

Q_DECLARE_METATYPE(CanvasGradient)


class ImageData {
};

class QContext2DCanvas;

class Context2D : public QObject
{
    Q_OBJECT

    // compositing
    Q_PROPERTY(qreal globalAlpha READ globalAlpha WRITE setGlobalAlpha)
    Q_PROPERTY(QString globalCompositeOperation READ globalCompositeOperation WRITE setGlobalCompositeOperation)
    Q_PROPERTY(QVariant strokeStyle READ strokeStyle WRITE setStrokeStyle)
    Q_PROPERTY(QVariant fillStyle READ fillStyle WRITE setFillStyle)
    // line caps/joins
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(QString lineCap READ lineCap WRITE setLineCap)
    Q_PROPERTY(QString lineJoin READ lineJoin WRITE setLineJoin)
    Q_PROPERTY(qreal miterLimit READ miterLimit WRITE setMiterLimit)
    // shadows
    Q_PROPERTY(qreal shadowOffsetX READ shadowOffsetX WRITE setShadowOffsetX)
    Q_PROPERTY(qreal shadowOffsetY READ shadowOffsetY WRITE setShadowOffsetY)
    Q_PROPERTY(qreal shadowBlur READ shadowBlur WRITE setShadowBlur)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor)

public:
    Context2D(QContext2DCanvas *parent);
    void setSize(int w, int h);
    
    void begin();
    const QPixmap & end();

    void clear();

    // compositing
    qreal globalAlpha() const; // (default 1.0)
    QString globalCompositeOperation() const; // (default over)
    QVariant strokeStyle() const; // (default black)
    QVariant fillStyle() const; // (default black)

    void setGlobalAlpha(qreal alpha);
    void setGlobalCompositeOperation(const QString &op);
    void setStrokeStyle(const QVariant &style);
    void setFillStyle(const QVariant &style);

    // line caps/joins
    qreal lineWidth() const; // (default 1)
    QString lineCap() const; // "butt", "round", "square" (default "butt")
    QString lineJoin() const; // "round", "bevel", "miter" (default "miter")
    qreal miterLimit() const; // (default 10)

    void setLineWidth(qreal w);
    void setLineCap(const QString &s);
    void setLineJoin(const QString &s);
    void setMiterLimit(qreal m);

    // shadows
    qreal shadowOffsetX() const; // (default 0)
    qreal shadowOffsetY() const; // (default 0)
    qreal shadowBlur() const; // (default 0)
    QColor shadowColor() const; // (default black)

    void setShadowOffsetX(qreal x);
    void setShadowOffsetY(qreal y);
    void setShadowBlur(qreal b);
    void setShadowColor(const QColor &c);

public slots:
    void save(); // push state on state stack
    void restore(); // pop state stack and restore state

    void scale(qreal x, qreal y);
    void rotate(qreal angle);
    void translate(qreal x, qreal y);
    void transform(qreal m11, qreal m12, qreal m21, qreal m22,
                   qreal dx, qreal dy);
    void setTransform(qreal m11, qreal m12, qreal m21, qreal m22,
                      qreal dx, qreal dy);

    CanvasGradient createLinearGradient(qreal x0, qreal y0,
                                        qreal x1, qreal y1);
    CanvasGradient createRadialGradient(qreal x0, qreal y0,
                                        qreal r0, qreal x1,
                                        qreal y1, qreal r1);

    // rects
    void clearRect(qreal x, qreal y, qreal w, qreal h);
    void fillRect(qreal x, qreal y, qreal w, qreal h);
    void strokeRect(qreal x, qreal y, qreal w, qreal h);

    // path API
    void beginPath();
    void closePath();
    void moveTo(qreal x, qreal y);
    void lineTo(qreal x, qreal y);
    void quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y);
    void bezierCurveTo(qreal cp1x, qreal cp1y,
                       qreal cp2x, qreal cp2y, qreal x, qreal y);
    void arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal radius);
    void rect(qreal x, qreal y, qreal w, qreal h);
    void arc(qreal x, qreal y, qreal radius,
             qreal startAngle, qreal endAngle,
             bool anticlockwise);
    void fill();
    void stroke();
    void clip();
    bool isPointInPath(qreal x, qreal y) const;

    // drawing images
    void drawImage(DomImage *image, qreal dx, qreal dy);
    void drawImage(DomImage *image, qreal dx, qreal dy,
                   qreal dw, qreal dh);
    void drawImage(DomImage *image, qreal sx, qreal sy,
                   qreal sw, qreal sh, qreal dx, qreal dy,
                   qreal dw, qreal dh);

    // pixel manipulation
    ImageData getImageData(qreal sx, qreal sy, qreal sw, qreal sh);
    void putImageData(ImageData image, qreal dx, qreal dy);
private:
    QPixmap  m_cache;
    QPainter m_painter;
    QPainterPath m_path;
    struct State {
        State()
            : creatingShape(false)
        {}

        QMatrix      matrix;
        bool         creatingShape;
    };
    State m_state;
    QStack<State> m_stateStack;
};

#endif
