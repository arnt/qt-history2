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

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#include "QtCore/qobject.h"

#ifndef QT_NO_SIGNALMAPPER
class QSignalMapperPrivate;

class Q_CORE_EXPORT QSignalMapper : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSignalMapper)
public:
    explicit QSignalMapper(QObject *parent = 0);
    ~QSignalMapper();

    void setMapping(const QObject *sender, int id);
    void setMapping(const QObject *sender, const QString &id);
    void removeMappings(const QObject *sender);

    QObject *mapping(int id) const;
    QObject *mapping(const QString &id) const;

signals:
    void mapped(int);
    void mapped(const QString &);

public slots:
    void map();

private slots:
    void removeMapping();

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QSignalMapper(QObject *parent, const char *name);
#endif
};
#endif // QT_NO_SIGNALMAPPER

#endif // QSIGNALMAPPER_H
