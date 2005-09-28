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

#ifndef QSVGSTYLE_P_H
#define QSVGSTYLE_P_H

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

#include "QtGui/qpen.h"
#include "QtGui/qbrush.h"
#include "QtGui/qmatrix.h"
#include "QtGui/qcolor.h"
#include "QtGui/qfont.h"

class QPainter;
class QSvgNode;

class QSvgStyleProperty
{
public:
    enum Type
    {
        QUALITY,
        FILL,
        VIEWPORT_FILL,
        FONT,
        STROKE,
        SOLID_COLOR,
        GRADIENT,
        TRANSFORM
    };
public:
    virtual ~QSvgStyleProperty();
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node)  =0;
    virtual void revert(QPainter *p) =0;
    virtual Type type() const=0;
};

class QSvgQualityStyle : public QSvgStyleProperty
{
public:
    QSvgQualityStyle(int color);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;
private:
    // color-render ing v 	v 	'auto' | 'optimizeSpeed' |
    //                                  'optimizeQuality' | 'inherit'
    int m_colorRendering;

    // shape-rendering v 	v 	'auto' | 'optimizeSpeed' | 'crispEdges' |
    //                                  'geometricPrecision' | 'inherit'
    //QSvgShapeRendering m_shapeRendering;


    // text-rendering    v 	v 	'auto' | 'optimizeSpeed' | 'optimizeLegibility'
    //                                | 'geometricPrecision' | 'inherit'
    //QSvgTextRendering m_textRendering;


    // vector-effect         v 	x 	'default' | 'non-scaling-stroke' | 'inherit'
    //QSvgVectorEffect m_vectorEffect;

    // image-rendering v 	v 	'auto' | 'optimizeSpeed' | 'optimizeQuality' |
    //                                      'inherit'
    //QSvgImageRendering m_imageRendering;
};

class QSvgFillStyle : public QSvgStyleProperty
{
public:
    QSvgFillStyle(const QBrush &brush);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QBrush & qbrush() const
    {
        return m_fill;
    }
private:
    // fill            v 	v 	'inherit' | <Paint.datatype>
    // fill-opacity    v 	v 	'inherit' | <OpacityValue.datatype>
    QBrush m_fill;
    // fill-rule       v 	v 	'inherit' | <ClipFillRule.datatype>
    //int m_fillRule;

    QBrush m_oldFill;
};

class QSvgViewportFillStyle : public QSvgStyleProperty
{
public:
    QSvgViewportFillStyle(const QBrush &brush);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QBrush & qbrush() const
    {
        return m_viewportFill;
    }
private:
    // viewport-fill         v 	x 	'inherit' | <Paint.datatype>
    // viewport-fill-opacity 	v 	x 	'inherit' | <OpacityValue.datatype>
    QBrush m_viewportFill;

    QBrush m_oldFill;
};

class QSvgFontStyle : public QSvgStyleProperty
{
public:
    QSvgFontStyle(const QFont &font);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QFont & qfont() const
    {
        return m_font;
    }
private:
    // font-family     v 	v 	'inherit' | <FontFamilyValue.datatype>
    // font-size       v 	v 	'inherit' | <FontSizeValue.datatype>
    // font-style      v 	v 	'normal' | 'italic' | 'oblique' | 'inherit'
    // font-weight     v 	v 	'normal' | 'bold' | 'bolder' | 'lighter' |
    //                                  '100' | '200' | '300' | '400' | '500' |
    //                                  '600' | '700' | '800' | '900' |
    //                                  'inherit'
    QFont m_font;

    QFont m_oldFont;
};

class QSvgStrokeStyle : public QSvgStyleProperty
{
public:
    QSvgStrokeStyle(const QPen &pen);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QPen & qpen() const
    {
        return m_stroke;
    }
private:
    // stroke            v 	v 	'inherit' | <Paint.datatype>
    // stroke-dasharray  v 	v 	'inherit' | <StrokeDashArrayValue.datatype>
    // stroke-dashoffset v 	v 	'inherit' | <StrokeDashOffsetValue.datatype>
    // stroke-linecap    v 	v 	'butt' | 'round' | 'square' | 'inherit'
    // stroke-linejoin   v 	v 	'miter' | 'round' | 'bevel' | 'inherit'
    // stroke-miterlimit v 	v 	'inherit' | <StrokeMiterLimitValue.datatype>
    // stroke-opacity    v 	v 	'inherit' | <OpacityValue.datatype>
    // stroke-width      v 	v 	'inherit' | <StrokeWidthValue.datatype>
    QPen m_stroke;

    QPen m_oldStroke;
};


class QSvgSolidColorStyle : public QSvgStyleProperty
{
public:
    QSvgSolidColorStyle(const QColor &color);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QColor & qcolor() const
    {
        return m_solidColor;
    }
private:
    // solid-color       v 	x 	'inherit' | <SVGColor.datatype>
    // solid-opacity     v 	x 	'inherit' | <OpacityValue.datatype>
    QColor m_solidColor;

    QBrush m_oldFill;
    QPen   m_oldStroke;
};

class QSvgGradientStyle : public QSvgStyleProperty
{
public:
    QSvgGradientStyle(QGradient *grad, bool resolveBounds=false);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    QGradient *qgradient() const
    {
        return m_gradient;
    }

    void addResolve(qreal offset);
private:
    QGradient      *m_gradient;
    QList<qreal>  m_resolvePoints;

    QBrush m_oldFill;
    bool   m_resolveBounds;
};

class QSvgTransformStyle : public QSvgStyleProperty
{
public:
    QSvgTransformStyle(const QMatrix &trans);
    virtual void apply(QPainter *p, const QRectF &, QSvgNode *node);
    virtual void revert(QPainter *p);
    virtual Type type() const;

    const QMatrix & qmatrix() const
    {
        return m_transform;
    }
private:
    //7.6 The transform  attribute
    QMatrix m_transform;

    QMatrix m_oldWorldMatrix;
};

class QSvgStyle
{
public:
    QSvgStyle()
        : quality(0),
          fill(0),
          viewportFill(0),
          font(0),
          stroke(0),
          solidColor(0),
          gradient(0),
          transform(0)
    {}
    void apply(QPainter *p, const QRectF &rect, QSvgNode *node);
    void revert(QPainter *p);
    QSvgQualityStyle       *quality;
    QSvgFillStyle          *fill;
    QSvgViewportFillStyle  *viewportFill;
    QSvgFontStyle          *font;
    QSvgStrokeStyle        *stroke;
    QSvgSolidColorStyle    *solidColor;
    QSvgGradientStyle      *gradient;
    QSvgTransformStyle     *transform;
};

/********************************************************/
// NOT implemented:

// color           v 	v 	'inherit' | <Color.datatype>
//QColor m_color;

// display         v 	x 	'inline' | 'block' | 'list-item'
//                                 | 'run-in' | 'compact' | 'marker' |
//                                 'table' | 'inline-table' |
//                                 'table-row-group' | 'table-header-group' |
//                                 'table-footer-group' | 'table-row' |
//                                 'table-column-group' | 'table-column' |
//                                 'table-cell' | 'table-caption' |
//                                 'none' | 'inherit'
//QSvgDisplayStyle m_display;

// display-align   v 	v 	'auto' | 'before' | 'center' | 'after' | 'inherit'
//QSvgDisplayAlign m_displayAlign;

// line-increment  v 	v 	'auto' | 'inherit' | <Number.datatype>
//int m_lineIncrement;

// text-anchor       v 	v 	'start' | 'middle' | 'end' | 'inherit'
//QSvgTextAnchor m_textAnchor;

// visibility 	v 	v 	'visible' | 'hidden' | 'inherit'
//QSvgVisibility m_visibility;

/******************************************************/
// the following do not make sense for us

// pointer-events  v 	v 	'visiblePainted' | 'visibleFill' | 'visibleStroke' |
//                              'visible' | 'painted' | 'fill' | 'stroke' | 'all' |
//                              'none' | 'inherit'
//QSvgPointEvents m_pointerEvents;

// audio-level     v  	x  	'inherit' | <Number.datatype>

#endif // QSVGSTYLE_P_H
