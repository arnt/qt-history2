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

#ifndef QAXBINDABLE_H
#define QAXBINDABLE_H

#include <QtGui/qwidget.h>

class QAxAggregated;
struct IAxServerBase;
struct IUnknown;

class QAxBindable
{
    friend class QAxServerBase;
public:
    QAxBindable();
    virtual ~QAxBindable();

    virtual QAxAggregated *createAggregate();
    static void reportError(int code, const QString &src, const QString &desc, const QString &help = QString());

protected:
    bool requestPropertyChange(const char *property);
    void propertyChanged(const char *property);

    IUnknown *clientSite() const;

private:
    IAxServerBase *activex;
};

#endif // QAXBINDABLE_H
