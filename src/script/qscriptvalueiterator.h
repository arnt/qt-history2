/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTVALUEITERATOR_H
#define QSCRIPTVALUEITERATOR_H

#include <QtScript/qscriptvalue.h>

QT_BEGIN_HEADER

QT_MODULE(Script)

class QString;

class QScriptValueIteratorPrivate;
class Q_SCRIPT_EXPORT QScriptValueIterator
{
public:
    QScriptValueIterator(const QScriptValue &value);
    ~QScriptValueIterator();

    bool hasNext() const;
    QString next();

    bool hasPrevious() const;
    QString previous();

    QString name() const;

    QScriptValue value() const;
    void setValue(const QScriptValue &value);

    QScriptValue::PropertyFlags flags() const;

    void remove();

    void toFront();
    void toBack();

    QScriptValueIterator& operator=(QScriptValue &value);

private:
    QScriptValueIteratorPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptValueIterator)
    Q_DISABLE_COPY(QScriptValueIterator)
};

QT_END_HEADER

#endif // QSCRIPTVALUEITERATOR_H
