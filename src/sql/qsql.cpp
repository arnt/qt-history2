/****************************************************************************
**
** Implementation of QSql class
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


/*!
  \class QSql qsql.h
  \ingroup database
  \mainclass
  \module sql

  \brief The QSql class is a namespace for Qt SQL identifiers that
  need to be global-like.

  Normally, you can ignore this class.  Several Qt SQL classes inherit
  it, so all the identifiers in the Qt SQL namespace are visible to
  you without qualification.

*/

/*! \enum QSql::Confirm

  This enum type describes edit confirmations.

  The currently defined values are:

  \value Yes
  \value No
  \value Cancel
*/

/*! \enum QSql::Op

  This enum type describes edit operations.

  The currently defined values are:

  \value None
  \value Insert
  \value Update
  \value Delete
*/


/*! \enum QSql::Location

  This enum type describes SQL navigation locations.

  The currently defined values are:

  \value BeforeFirst
  \value AfterLast
*/

/*! \fn QSql::QSql()
  Constructs a Qt SQL namepsace class
*/
