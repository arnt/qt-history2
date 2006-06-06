/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qhiviewwidget_mac_p.h>
#include <qdebug.h>

extern HIViewRef qt_mac_hiview_for(const QWidget *w); //qwidget_mac.cpp
extern HIViewRef qt_mac_hiview_for(WindowPtr w); //qwidget_mac.cpp

QHIViewWidget::QHIViewWidget(WindowRef windowref, bool createSubs, QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    create((WId)qt_mac_hiview_for(windowref), false);
    Rect rect;
    GetWindowBounds(windowref, kWindowContentRgn, &rect);
    setGeometry(QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));
    if(createSubs)
        createQWidgetsFromHIViews();
}

QHIViewWidget::~QHIViewWidget()
{
}

QHIViewWidget::QHIViewWidget(HIViewRef hiviewref, bool createSubs, QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    create((WId)hiviewref, false);
    setVisible(HIViewIsVisible(hiviewref));
    if(createSubs)
        createQWidgetsFromHIViews();
}

void QHIViewWidget::createQWidgetsFromHIViews()
{
    // Nicely walk through and make HIViewWidget out of all the children
    addViews_recursive(HIViewGetFirstSubview(qt_mac_hiview_for(this)), this);
}

void QHIViewWidget::addViews_recursive(HIViewRef child, QWidget *parent)
{
    if (!child)
        return;
    QWidget *widget = QWidget::find(WId(child));
    if(!widget)
        widget = new QHIViewWidget(child, false, parent);
    addViews_recursive(HIViewGetFirstSubview(child), widget);
    addViews_recursive(HIViewGetNextView(child), parent);
}
