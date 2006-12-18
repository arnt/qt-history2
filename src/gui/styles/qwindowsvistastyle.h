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

#ifndef QWINDOWSVISTASTYLE_H
#define QWINDOWSVISTASTYLE_H

#include <QtGui/qwindowsxpstyle.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_STYLE_WINDOWSVISTA)

class QWindowsVistaStylePrivate;
class Q_GUI_EXPORT QWindowsVistaStyle : public QWindowsXPStyle
{
    Q_OBJECT
public:
    QWindowsVistaStyle();
    
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = 0) const;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;

    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget) const;
    
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
				     const QPoint &pos, const QWidget *widget = 0) const;

    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0,
                  QStyleHintReturn *returnData = 0) const;

    
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette pal);
    void polish(QApplication *app);
    void unpolish(QApplication *app);
    QPalette standardPalette();

private:
    Q_DISABLE_COPY(QWindowsVistaStyle)
    Q_DECLARE_PRIVATE(QWindowsVistaStyle)
    friend class QStyleFactory;
};
#endif //QT_NO_STYLE_WINDOWSVISTA

QT_END_HEADER

#endif //QWINDOWSVISTASTYLE_H
