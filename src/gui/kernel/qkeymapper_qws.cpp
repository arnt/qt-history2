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

#include "qkeymapper_p.h"
#include <qdebug.h>
#include <private/qevent_p.h>
#include <private/qlocale_p.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE


QKeyMapperPrivate::QKeyMapperPrivate()
{
    keyboardInputLocale = QLocale::system();
    keyboardInputDirection = Qt::RightToLeft;
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
    // clearMappings();
}

void QKeyMapperPrivate::clearMappings()
{
}

QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *e)
{
    QList<int> result;
    if (e->key() && (e->key() != Qt::Key_unknown))
        result << int(e->key() + e->modifiers());
    else if (!e->text().isEmpty())
        result << int(e->text().at(0).unicode() + e->modifiers());
    return result;
}

QT_END_NAMESPACE
