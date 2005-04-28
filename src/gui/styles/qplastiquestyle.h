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

#ifndef QPLASTIQUESTYLE_H
#define QPLASTIQUESTYLE_H

#include <QtGui/qwindowsstyle.h>


#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_PLASTIQUE
#else
#define Q_GUI_EXPORT_STYLE_PLASTIQUE Q_GUI_EXPORT
#endif


class QPlastiqueStylePrivate;
class Q_GUI_EXPORT_STYLE_PLASTIQUE QPlastiqueStyle : public QWindowsStyle
{
public:
    QPlastiqueStyle();
    ~QPlastiqueStyle();

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

    int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0,
		  QStyleHintReturn *returnData = 0) const;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
				     const QPoint &pos, const QWidget *widget = 0) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

    void polish(QWidget *widget);
    void polish(QApplication *app);
    void polish(QPalette &pal);

    QPalette standardPalette();

private:
    QPlastiqueStylePrivate *d;
};

#endif
