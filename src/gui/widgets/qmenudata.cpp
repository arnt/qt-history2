/****************************************************************************
**
** Definition of QWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/#include "qmenudata.h"

#ifdef QT_COMPAT
#include <qaction.h>
#include <private/qaction_p.h>
#define d d_func()

QMenuItem::QMenuItem() : QAction((QWidget*)0)
{
}
 
QMenuItem::~QMenuItem()
{
}

void QMenuItem::setId(int id)
{
    d->param = d->id = id;
}

int QMenuItem::id() const
{
    return d->id;
}

void QMenuItem::setSignalValue(int param)
{
    d->param = param;
}

int QMenuItem::signalValue() const
{
    return d->param;
}
#endif
