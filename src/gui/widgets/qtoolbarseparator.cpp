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

#include "qtoolbarseparator_p.h"

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qpainter.h>

static QStyleOption getStyleOption(const QToolBarSeparator *tbs)
{
    QStyleOption opt(0, QStyleOption::SO_Default);
    opt.rect = tbs->rect();
    opt.palette = tbs->palette();
    if (tbs->orientation() == Qt::Horizontal)
        opt.state = QStyle::Style_Horizontal;
    else
        opt.state = QStyle::Style_Default;
    return opt;
}

QToolBarSeparator::QToolBarSeparator(Qt::Orientation orientation, QToolBar *parent)
    : QWidget(parent), orient(orientation)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

QSize QToolBarSeparator::sizeHint() const
{
    QStyleOption opt = getStyleOption(this);
    const int extent = style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent, &opt, this);
    return QSize(extent, extent);
}

void QToolBarSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt = getStyleOption(this);
    style().drawPrimitive(QStyle::PE_DockWindowSeparator, &opt, &p, this);
}
