/****************************************************************************
**
** Implementation of QSql class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


/*!
    \class QSql qsql.h
    \brief The QSql class is a namespace for Qt SQL identifiers that
    need to be global-like.

    \ingroup database
    \mainclass
    \module sql

    Normally, you can ignore this class. Several Qt SQL classes
    inherit it, so all the identifiers in the Qt SQL namespace are
    visible without qualification.
*/

/*!
    \enum QSql::Confirm

    This enum type describes edit confirmations.

    \value Yes
    \value No
    \value Cancel
*/

/*!
    \enum QSql::Op

    This enum type describes edit operations.

    \value None
    \value Insert
    \value Update
    \value Delete
*/


/*!
    \enum QSql::Location

    This enum type describes SQL navigation locations.

    \value BeforeFirst
    \value AfterLast
*/

/*!
    \enum QSql::ParamTypeFlags

    This enum is used to set the type of a bind parameter

    \value In  the bind parameter is used to put data into the database
    \value Out  the bind parameter is used to receive data from the database
    \value InOut  the bind parameter is used to put data into the
        database; it will be overwritten with output data on executing
        a query.
    \value Binary this must be OR'd with one of the other flags if you
    want to indicate that the data being transferred is raw binary data
*/

/*!
    \enum QSql::TableType

    This enum type describes types of tables

    \value Tables  All the tables visible to the user
    \value SystemTables  Internal tables used by the database
    \value Views  All the views visible to the user
    \value AllTables  All of the above
*/

/*!
    \fn QSql::QSql()

    \internal

    Constructs a Qt SQL namespace class
*/
