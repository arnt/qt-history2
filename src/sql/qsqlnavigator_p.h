/****************************************************************************
**
** Definition of some Qt private functions.
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

#ifndef QSQLNAVIGATOR_P_H
#define QSQLNAVIGATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_SQLFORMNAV_CHILD
#error "Undefined child class while implementing forwarding functions"
#endif

#define QT_SQLFORMNAV_FWD( r, c, m, proto, parm ) \
r c::m##proto { QSqlFormNavigator::m(parm); }

#define QT_SQLFORMNAV_RET_FWD( r, c, m, proto, parm ) \
r c::m##proto { return QSqlFormNavigator::m(parm); }

#define QT_SQLFORMNAV_DIF_FWD( r, c, m, n, p ) \
r c::m##p { QSqlFormNavigator::n##p; }

#define QT_SQLFORMNAV_DIF_RET_FWD( r, c, m, n, p ) \
r c::m##p { return QSqlFormNavigator::n##p; }


QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, setBoundryChecking, ( bool active ), active )
QT_SQLFORMNAV_RET_FWD( bool, QT_SQLFORMNAV_CHILD, boundryChecking, () const,  )

QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, setSort, ( const QStringList& sort ), sort )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, setSort, ( const QSqlIndex& sort ), sort )
QT_SQLFORMNAV_RET_FWD( QStringList, QT_SQLFORMNAV_CHILD, sort, () const,  )

QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, setFilter, ( const QString& filter ), filter )
QT_SQLFORMNAV_RET_FWD( QString, QT_SQLFORMNAV_CHILD, filter, () const,  )

QT_SQLFORMNAV_DIF_FWD( void, QT_SQLFORMNAV_CHILD, insertRecord, insert, () )
QT_SQLFORMNAV_DIF_FWD( void, QT_SQLFORMNAV_CHILD, updateRecord, update, () )
QT_SQLFORMNAV_DIF_FWD( void, QT_SQLFORMNAV_CHILD, deleteRecord, del, () )

QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, firstRecord, (),  )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, lastRecord, (),  )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, nextRecord, (),  )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, prevRecord, (),  )

QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, readFields, (), )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, writeFields, (), )
QT_SQLFORMNAV_FWD( void, QT_SQLFORMNAV_CHILD, clearFormValues, (), )

#undef QT_SQLFORMNAV_CHILD

#endif
