/****************************************************************************
**
** Implementation of QSqlWidget class
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

#include "qsqlwidget.h"

#ifndef QT_NO_SQL

#include "qsqlform.h"

/* ATTENTION: this file must remain in sync with qsqldialog.cpp */

/*!

  \class QSqlWidget qsqlwidget.h
  \brief SQL cursor/form manipulation and navigation

  \module sql

  This class is used to manipulate and navigate data entry forms.  A
  high-level API is provided to navigate through data records in a
  cursor, insert, update and delete records, and refresh data in the
  display.

  Instances of this class cannot be created directly.  Derived classes
  must reimplement certain functions (see
  QSqlFormNavigator::defaultForm() and QSqlNavigator::defaultCursor())
  which provide the form and cursor on which the navigator operates.

  Convenient signals and slots are provided to navigate the cursor
  (see firstRecord(), lastRecord(), prevRecord(), nextRecord()), to
  update records (see insertRecord(), updateRecord(), deleteRecord()),
  and to update the display according to the cursor's current position
  (see firstRecordAvailable(), lastRecordAvailable(),
  nextRecordAvailable(), prevRecordAvailable()).

*/

QSqlWidget::QSqlWidget( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, fl ), QSqlFormNavigator()
{
}

// implement forwarding funcs
#define QT_SQLFORMNAV_CHILD QSqlWidget
#include "qsqlnavigator_p.h"

#endif
