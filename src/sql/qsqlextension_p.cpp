/****************************************************************************
**
** Implementation of the QSqlExtension class
**
** Created : 2002-06-03
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsqlextension_p.h"

#ifndef QT_NO_SQL
QSqlExtension::QSqlExtension()
    : bindm( BindByPosition )
{
}

QSqlExtension::~QSqlExtension()
{
}

bool QSqlExtension::prepare( const QString& /*query*/ )
{
    clearValues();
    return FALSE;
}

bool QSqlExtension::exec()
{
    return FALSE;
}

void QSqlExtension::bindValue( const QString& placeholder, const QVariant& val )
{
    bindm = BindByName;
    values[ placeholder ] = val;
}

void QSqlExtension::bindValue( int pos, const QVariant& val )
{
    bindm = BindByPosition;
    index[ pos ] = QString::number( pos );
    values[ QString::number( pos ) ] = val;
}

void QSqlExtension::addBindValue( const QVariant& val )
{
    bindm = BindByPosition;
    bindValue( index.count(), val );
}

void QSqlExtension::clearValues()
{
    index.clear();
    values.clear();
}

QSqlExtension::BindMethod QSqlExtension::bindMethod()
{
    return bindm;
}
#endif
