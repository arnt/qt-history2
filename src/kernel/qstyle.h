/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.h#6 $
**
** Definition of QStyle class
**
** Created : 980616
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QStyle
{
    GUIStyle gs;
public:
    QStyle(GUIStyle);
    QStyle();
    virtual ~QStyle();

#ifndef NO_QT1_COMPAT
    operator GUIStyle() const { return gs; }
    int operator==(GUIStyle s) const { return gs==s; }
    int operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }

    virtual void initialize( QApplication*);

    virtual void polish( QWidget* );

    virtual QRect itemRect( QPainter *p, int x, int y, int w, int h,
		    int flags, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len=-1 );

    virtual void drawItem( QPainter *p, int x, int y, int w, int h,
		    int flags, const QColorGroup &g, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len=-1 );


    virtual void drawSeparator( QPainter *p, int x1, int y1, int x2, int y2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

    virtual void drawRect( QPainter *p, int x, int y, int w, int h,
		    const QColor &, int lineWidth = 1,
		    const QBrush *fill = 0 );

    virtual void drawRectStrong( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

    virtual void drawButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

    virtual void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    int lineWidth = 1, const QBrush *fill = 0 );

    enum ArrowType { UpArrow, DownArrow, LeftArrow, RightArrow };

    virtual void drawArrow( QPainter *p, ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled );

    // "radio button"
    virtual QSize exclusiveIndicatorSize() const;
    virtual void drawExclusiveIndicator( QPainter* /*translated*/,
		    const QColorGroup &g, bool on, bool down );

    // "check box"
    virtual QSize indicatorSize() const;
    virtual void drawIndicator( QPainter* /*translated*/,
		    const QColorGroup &g, bool on, bool down );

    virtual void drawFocusRect( QPainter*,
		    const QRect&, const QColorGroup & );
};

class Q_EXPORT QHStyle : public QStyle
{
public:
    QHStyle(GUIStyle);
    void initialize( QApplication*);
    void polish( QWidget* );
};

#endif // QSTYLE_H
