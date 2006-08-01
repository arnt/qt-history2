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

#ifndef QSVGGRAPHICS_P_H
#define QSVGGRAPHICS_P_H

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

#include "qsvgnode_p.h"

#include "QtGui/qpainterpath.h"
#include "QtGui/qimage.h"
#include "QtGui/qtextlayout.h"
#include "QtGui/qtextoption.h"
#include "QtCore/qstack.h"

class QTextCharFormat;

class QSvgAnimation : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

class QSvgArc : public QSvgNode
{
public:
    QSvgArc(QSvgNode *parent, const QPainterPath &path);
    virtual void draw(QPainter *p);
    virtual Type type() const;
private:
    QPainterPath cubic;
};

class QSvgCircle : public QSvgNode
{
public:
    QSvgCircle(QSvgNode *parent, const QRectF &rect);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QRectF m_bounds;
};

class QSvgEllipse : public QSvgNode
{
public:
    QSvgEllipse(QSvgNode *parent, const QRectF &rect);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QRectF m_bounds;
};

class QSvgImage : public QSvgNode
{
public:
    QSvgImage(QSvgNode *parent, const QImage &image,
              const QRect &bounds);
    virtual void draw(QPainter *p);
    virtual Type type() const;
private:
    QImage m_image;
    QRect  m_bounds;
};

class QSvgLine : public QSvgNode
{
public:
    QSvgLine(QSvgNode *parent, const QLineF &line);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    //virtual QRectF bounds() const;
private:
    QLineF m_bounds;
};

class QSvgPath : public QSvgNode
{
public:
    QSvgPath(QSvgNode *parent, const QPainterPath &qpath);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QPainterPath m_path;
    QRectF m_cachedBounds;
};

class QSvgPolygon : public QSvgNode
{
public:
    QSvgPolygon(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QPolygonF m_poly;
};

class QSvgPolyline : public QSvgNode
{
public:
    QSvgPolyline(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p);
    virtual Type type() const;
private:
    QPolygonF m_poly;
};

class QSvgRect : public QSvgNode
{
public:
    QSvgRect(QSvgNode *paren, const QRectF &rect, int rx=0, int ry=0);
    virtual Type type() const;
    virtual void draw(QPainter *p);
    virtual QRectF bounds() const;
private:
    QRectF m_rect;
    int m_rx, m_ry;
};

class  QSvgText : public QSvgNode
{
public:
    QSvgText(QSvgNode *parent, const QPointF &coord);
    ~QSvgText();
    virtual void draw(QPainter *p);
    virtual Type type() const;
    void insertText(const QString &text);
    void insertFormat(const QTextCharFormat &format);
    void popFormat();
    void setTextAlignment(const Qt::Alignment &alignment);
    const QTextCharFormat &topFormat() const;
    //virtual QRectF bounds() const;
private:
    QPointF m_coord;

    QString m_text;
    QStack<QTextCharFormat> m_formats;
    Qt::Alignment           m_textAlignment;
    QList<QTextLayout::FormatRange> m_formatRanges;

};

class  QSvgTextArea : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

class QSvgUse : public QSvgNode
{
public:
    QSvgUse(const QPointF &start, QSvgNode *parent, QSvgNode *link);
    virtual void draw(QPainter *p);
    virtual Type type() const;
private:
    QSvgNode *m_link;
    QPointF   m_start;
};

class QSvgVideo : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

#endif // QSVGGRAPHICS_P_H
