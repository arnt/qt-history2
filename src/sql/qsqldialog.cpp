/****************************************************************************
**
** Implementation of QSqlDialog class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "qsqldialog.h"

#ifndef QT_NO_SQL

/* ATTENTION: this file must remain in sync with qsqlwidget.cpp */

QSqlDialog::QSqlDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f ), QSqlFormNavigator()
{
}

void QSqlDialog::setBoundryChecking( bool active )
{
    QSqlFormNavigator::setBoundryChecking( active );
}

bool QSqlDialog::boundryChecking() const
{
    return QSqlFormNavigator::boundryChecking();
}

void QSqlDialog::setSort( const QSqlIndex& sort )
{
    QSqlFormNavigator::setSort( sort );
}

void QSqlDialog::setSort( const QStringList& sort )
{
    QSqlFormNavigator::setSort( sort );
}

QStringList  QSqlDialog::sort() const
{
    return QSqlFormNavigator::sort();
}

void QSqlDialog::setFilter( const QString& filter )
{
    QSqlFormNavigator::setFilter( filter );
}

QString QSqlDialog::filter() const
{
    return QSqlFormNavigator::filter();
}

/*! \fn void firstRecordAvailable( bool available )
*/

/*! \fn void lastRecordAvailable( bool available )
*/

/*! \fn void nextRecordAvailable( bool available )
*/

/*! \fn void prevRecordAvailable( bool available )
*/

void QSqlDialog::insertRecord()
{
    QSqlFormNavigator::insertRecord();
}

void QSqlDialog::updateRecord()
{
    QSqlFormNavigator::updateRecord();
}

void QSqlDialog::deleteRecord()
{
    QSqlFormNavigator::deleteRecord();
}

void QSqlDialog::firstRecord()
{
    QSqlFormNavigator::firstRecord();
}

void QSqlDialog::lastRecord()
{
    QSqlFormNavigator::lastRecord();
}

void QSqlDialog::nextRecord()
{
    QSqlFormNavigator::nextRecord();
}

void QSqlDialog::prevRecord()
{
    QSqlFormNavigator::prevRecord();
}

void QSqlDialog::clearForm()
{
    QSqlFormNavigator::clearForm();
}

void QSqlDialog::emitFirstRecordAvailable( bool available )
{
    emit firstRecordAvailable( available );
}

void QSqlDialog::emitLastRecordAvailable( bool available )
{
    emit lastRecordAvailable( available );
}

void QSqlDialog::emitNextRecordAvailable( bool available )
{
    emit nextRecordAvailable( available );
}

void QSqlDialog::emitPrevRecordAvailable( bool available )
{
    emit prevRecordAvailable( available );
}

#endif
