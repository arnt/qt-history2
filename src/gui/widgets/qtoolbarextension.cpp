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

#include "qtoolbarextension_p.h"
#include <qpixmap.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <qstylepainter.h>


#ifndef QT_NO_TOOLBUTTON

QToolBarExtension::QToolBarExtension(QWidget *parent)
    : QToolButton(parent)
{
    setObjectName(QLatin1String("qt_toolbar_ext_button"));
    setAutoRaise(true);
#ifndef QT_NO_MENU
    setPopupMode(QToolButton::InstantPopup);
#endif
    setOrientation(Qt::Horizontal);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void QToolBarExtension::setOrientation(Qt::Orientation o)
{
    if (o == Qt::Horizontal) {
        setIcon(style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
    } else {
        setIcon(style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton));
   }
}

QSize QToolBarExtension::sizeHint() const
{
    int ext = style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    return QSize(ext, ext);
}

void QToolBarExtension::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    //remove menu arrow from tool button
    opt.features &= ~QStyleOptionToolButton::Menu;
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

#endif // QT_NO_TOOLBUTTON
