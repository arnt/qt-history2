/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <qobject.h>
#include <qdatetime.h>
#include <qtimer.h>

class TimeStamp : public QObject
{
    Q_OBJECT

public:
    TimeStamp( QObject *parent, const QString &f );

    void setFileName( const QString &f );
    QString fileName() const;
    void setAutoCheckEnabled( bool a );
    void update();

    bool isUpToDate() const;
    bool isAutoCheckEnabled() const;

signals:
    void timeStampChanged();

private slots:
    void autoCheckTimeStamp();

private:
    QDateTime lastTimeStamp;
    QString filename;
    bool autoCheck;
    QTimer *timer;

};

#endif
