/****************************************************************************
**
** Definition of shared Qt SQL module classes
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
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

#ifndef QSQLCACHEDRESULT_H
#define QSQLCACHEDRESULT_H

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

#ifndef QT_NO_SQL

#include <qglobal.h>
#include <qvariant.h>
#include <qvector.h>
#include <qsqlresult.h>

class QtSqlCachedResultPrivate;

class Q_SQL_EXPORT QtSqlCachedResult: public QSqlResult
{
public:
    virtual ~QtSqlCachedResult();

    typedef QVector<QCoreVariant> ValueCache;

protected:
    QtSqlCachedResult(const QSqlDriver * db);

    void init(int colCount);
    void cleanup();

    virtual bool gotoNext(ValueCache &values, int index) = 0;

    QCoreVariant data(int i);
    bool isNull(int i);
    bool fetch(int i);
    bool fetchNext();
    bool fetchPrev();
    bool fetchFirst();
    bool fetchLast();

    int colCount() const;
    ValueCache &cache();

private:
    bool cacheNext();
    QtSqlCachedResultPrivate *d;
};


#endif

#endif
