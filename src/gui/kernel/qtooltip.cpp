/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtooltip.h"
#include "qlabel.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qapplication.h"
#include "qpointer.h"
#include "qtimer.h"
#include <private/qeffects_p.h>
#include "qdebug.h"

void QToolTip::showText(const QPoint &, const QString &s, QWidget *w)
{
    qDebug() << "QToolTip::showText" << s << "for" << w;
}

QPalette QToolTip::palette()
{
    return QApplication::palette("QTipLabel");
}

#include "qtooltip.moc"

