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

#ifndef QPOCKETPCSTYLE_WCE_H
#define QPOCKETPCSTYLE_WCE_H
//#ifndef QT_NO_STYLE_POCKETPC

#include "QtGui/qstyle.h"
#include "QtGui/qpalette.h"

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_POCKETPC
#else
#define Q_GUI_EXPORT_STYLE_POCKETPC Q_EXPORT
#endif

class QPainter;
class Q_GUI_EXPORT_STYLE_POCKETPC QPocketPCStyle : public QStyle
{
    Q_OBJECT
public:
    QPocketPCStyle();
    virtual ~QPocketPCStyle();

    virtual void polish(QApplication*);
    virtual void polish(QWidget*);
    virtual void unPolish(QApplication*);
    virtual void unpolish(QWidget*);

    // new stuff
    void drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r, const QPalette &pal, SFlags flags = Style_Default, const QStyleOption& = QStyleOption::Default) const;
    void drawControl(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r, const QPalette &pal, SFlags how = Style_Default, const QStyleOption& = QStyleOption::Default) const;
    void drawComplexControl(ComplexControl control, QPainter* p, const QWidget* widget, const QRect& r, const QPalette& pal, SFlags how = Style_Default, SCFlags sub = SC_All, SCFlags subActive = SC_None, const QStyleOption& = QStyleOption::Default) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl control, const QWidget *widget, SubControl sc, const QStyleOption& = QStyleOption::Default) const;
    QSize sizeFromContents(ContentsType contents, const QWidget *widget, const QSize &contentsSize, const QStyleOption& = QStyleOption::Default) const;
    QPixmap stylePixmap(StylePixmap stylepixmap, const QWidget *widget = 0, const QStyleOption& = QStyleOption::Default) const;
    QPixmap stylePixmap(PixmapType pixmapType, const QPixmap &pix, const QPalette &pal, const QStyleOption& = QStyleOption::Default) const;

    void drawControlMask(ControlElement,QPainter *,const QWidget *,const QRect &,const QStyleOption &) const;
    QRect subRect(SubRect,const QWidget *) const;
    void drawComplexControlMask(ComplexControl,QPainter *,const QWidget *,const QRect &,const QStyleOption &) const;
    SubControl hitTestComplexControl(ComplexControl,const QWidget *,const QPoint &,const QStyleOption &) const;
    int styleHint(StyleHint,const QWidget *,const QStyleOption &,QStyleHintReturn *) const;

private:
    Q_DISABLE_COPY(QPocketPCStyle)

    Qt::Dock findLocation(QWidget *p) const;
    Qt::Dock findLocation(QPainter *p) const;

#ifndef Q_OS_TEMP
    void modifyOriginalPalette();
    QPalette originalPal;
    bool gotOriginal;
#endif // Q_OS_TEMP
};

//#endif // QT_NO_STYLE_POCKETPC
#endif
