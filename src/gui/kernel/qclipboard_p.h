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

#ifndef QCLIPBOARD_P_H
#define QCLIPBOARD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qobject_p.h>
#include "qmime.h"
#include "qclipboard.h"

class QClipboardPrivate;

class QMimeDataWrapper : public QMimeSource
{
public:
    QMimeDataWrapper() {}

    const char* format(int n) const;
    QByteArray encodedData(const char*) const;

    mutable QList<QByteArray> formats;
    const QMimeData *data;
};

class QMimeSourceWrapper : public QMimeData
{
public:
    QMimeSourceWrapper(QClipboardPrivate *priv, QClipboard::Mode m);
    ~QMimeSourceWrapper();

    bool hasFormat(const QString &mimetype) const;
    QStringList formats() const;

protected:
    QCoreVariant retrieveData(const QString &mimetype, QCoreVariant::Type) const;
private:
    QClipboardPrivate *d;
    QClipboard::Mode mode;
    QMimeSource *source;
};


class QClipboardPrivate : public QObjectPrivate
{
public:
    QClipboardPrivate() : QObjectPrivate() {
        compat_data[0] = compat_data[1] = 0;
        wrapper[0] = new QMimeDataWrapper();
        wrapper[1] = new QMimeDataWrapper();
    }
    ~QClipboardPrivate() {
        delete wrapper[0];
        delete wrapper[1];
        delete compat_data[0];
        delete compat_data[1];
    }

    mutable QMimeDataWrapper *wrapper[2];
    mutable QMimeSource *compat_data[2];
};

inline QMimeSourceWrapper::QMimeSourceWrapper(QClipboardPrivate *priv, QClipboard::Mode m)
    : QMimeData()
{
    d = priv;
    mode = m;
    source = d->compat_data[mode];
}

inline QMimeSourceWrapper::~QMimeSourceWrapper()
{
    if (d->compat_data[mode] == source)
        d->compat_data[mode] = 0;
    delete source;
}

#endif // QCLIPBOARD_P_H
