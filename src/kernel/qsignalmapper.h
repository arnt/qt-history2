/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignalmapper.h#10 $
**
** Definition of QSignalMapper class
**
** Created : 980503
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class  QSignalMapperData;
struct QSignalMapperRec;


class Q_EXPORT QSignalMapper : public QObject {
    Q_OBJECT
public:
    QSignalMapper( QObject* parent, const char* name=0 );
    ~QSignalMapper();

    virtual void setMapping( const QObject* sender, int identifier );
    virtual void setMapping( const QObject* sender, const QString &identifier );
    void removeMappings( const QObject* sender );

signals:
    void mapped(int);
    void mapped(const QString &);

public slots:
    void map();

private:
    QSignalMapperData* d;
    QSignalMapperRec* getRec( const QObject* );

private slots:
    void removeMapping();
};


#endif // QSIGNALMAPPER_H
