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

/****************************************************************************
**
** Implementation of QMacInputContext class
**
** Copyright (C) 2003-2004 IM module for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#include <QtGui/qwidget.h>
#include <private/qmacinputcontext_p.h>

extern void qt_mac_unicode_reset_input(QWidget *); //qapplication_mac.cpp

QMacInputContext::QMacInputContext(QObject *parent)
    : QInputContext(parent)
{
}

QMacInputContext::~QMacInputContext()
{
}

QString QMacInputContext::language()
{
    return QString();
};

void QMacInputContext::reset()
{
    if (QWidget *w = qobject_cast<QWidget *>(parent()))
        qt_mac_unicode_reset_input(w);
}

bool QMacInputContext::isComposing() const
{
    return false;
}
