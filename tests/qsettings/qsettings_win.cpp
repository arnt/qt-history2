/****************************************************************************
**
**
** Implementation of QSettings class
**
** Created : 2000.07.14
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsettings.h"
#include "qxml.h"

#include <qvariant.h>

void QSettings::write()
{
}

QVariant QSettings::readEntry( const QString &key )
{
    return QVariant();
}

void QSettings::removeEntry( const QString &key )
{
}

void QSettings::writeEntry( const QString &key, const QVariant &value )
{
}

void QSettings::cleanup()
{
}