/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignalmapper.h#3 $
**
** Definition of QSignalMapper class
**
** Created : 980503
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class  QSignalMapperData;
struct QSignalMapperRec;


class QSignalMapper : public QObject {
    Q_OBJECT
public:
    QSignalMapper( QObject* parent, const char* name=0 );
    ~QSignalMapper();

    void setMapping( const QObject* sender, int identifier );
    void setMapping( const QObject* sender, const char* identifier );
    void removeMappings( const QObject* sender );

signals:
    void mapped(int);
    void mapped(const char*);

public slots:
    void map();

private:
    QSignalMapperData* d;
    QSignalMapperRec* getRec( const QObject* );

private slots:
    void removeMapping();
};


#endif // QSIGNALMAPPER_H
