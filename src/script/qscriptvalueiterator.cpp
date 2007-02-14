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

#include "qscriptvalueiterator.h"
#include "qscriptvalueiterator_p.h"
#include "qscriptvalue_p.h"
#include "qscriptmember_p.h"
#include "qscriptengine_p.h" // ### QScriptNameIdImpl, move in separate header file

/*!
  \since 4.3
  \class QScriptValueIterator

  \brief The QScriptValueIterator class provides a Java-style iterator for QScriptValue.

  \ingroup script
  \mainclass

  The QScriptValueIterator constructor takes a QScriptValue as
  argument.  After construction, the iterator is located at the very
  beginning of the sequence of properties. Here's how to iterate over
  all the properties of a QScriptValue:

  \code
  QScriptValue object;
  ...
  QScriptValueIterator it(object);
  while (it.hasNext()) {
    it.next();
    qDebug() << it.name() << ": " << it.value();
  }
  \endcode

  The next() function returns the name of the next property and
  advances the iterator. The name(), value() and flags() functions
  return the name, value and flags of the last item that was jumped
  over.

  If you want to remove properties as you iterate over the
  QScriptValue, use remove(). If you want to modify the value of a
  property, use setValue().

  Note that QScriptValueIterator only iterates over the QScriptValue's
  own properties; i.e. it does not follow the prototype chain.

  \sa QScriptValue::setProperty()
*/

/*!
  Constructs an iterator for traversing \a object. The iterator is
  set to be at the front of the sequence of properties (before the
  first property).
*/
QScriptValueIterator::QScriptValueIterator(const QScriptValue &object)
    : d_ptr(new QScriptValueIteratorPrivate(this, object))
{
}

/*!
  Destroys the iterator.
*/
QScriptValueIterator::~QScriptValueIterator()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns true if there is at least one item ahead of the iterator,
  i.e. the iterator is \i not at the back of the property sequence;
  otherwise returns false.
*/
bool QScriptValueIterator::hasNext() const
{
    Q_D(const QScriptValueIterator);
    if ((d->nextIndex != -1) && d->forward)
        return true;

    if (!d->object.isObject())
        return false;

    QScriptValueImpl v = QScriptValuePrivate::valueOf(d->object);
    int i = d->index;
    if ((i != -1) && !d->forward)
        --i;

    int count = v.memberCount();
    bool found = false;
    while (! found && ++i < count) {
        QScript::Member member;
        v.member(i, &member);
        found = member.isValid();
        if (found) {
            QScriptValueImpl vv;
            v.get(member, &vv);
            found = vv.isValid();
        }
    }

    QScriptValueIteratorPrivate *that;
    that = const_cast<QScriptValueIteratorPrivate*>(d);
    if (found) {
        that->forward = true;
        that->nextIndex = i;
        return true;
    } else {
        that->nextIndex = -1;
        return false;
    }
}

/*!
  Returns the name of the next property and advances the iterator by
  one position.
*/
QString QScriptValueIterator::next()
{
    Q_D(QScriptValueIterator);
    (void)hasNext();

    d->index = d->nextIndex;
    d->nextIndex = -1;

    return name();
}

/*!
  Returns true if there is at least one item behind the iterator,
  i.e. the iterator is \i not at the front of the property sequence;
  otherwise returns false.
*/
bool QScriptValueIterator::hasPrevious() const
{
    Q_D(const QScriptValueIterator);
    if ((d->nextIndex != -1) && !d->forward)
        return true;

    if (!d->object.isObject())
        return false;

    QScriptValueImpl v = QScriptValuePrivate::valueOf(d->object);
    int i = d->index;
    if ((i != -1) && d->forward)
        ++i;

    bool found = false;
    while (! found && --i >= 0) {
        QScript::Member member;
        v.member(i, &member);
        found = member.isValid();
        if (found) {
            QScriptValueImpl vv;
            v.get(member, &vv);
            found = vv.isValid();
        }
    }

    QScriptValueIteratorPrivate *that;
    that = const_cast<QScriptValueIteratorPrivate*>(d);
    if (found) {
        that->forward = false;
        that->nextIndex = i;
        return true;
    } else {
        that->nextIndex = -1;
        return false;
    }
    
}

/*!
  Returns the name of the previous property and moves the iterator
  back by one position.
*/
QString QScriptValueIterator::previous()
{
    Q_D(QScriptValueIterator);
    (void)hasPrevious();

    d->index = d->nextIndex;
    d->nextIndex = -1;

    return name();
}

/*!
  Moves the iterator to the front of the QScriptValue (before the
  first property).
*/
void QScriptValueIterator::toFront()
{
    Q_D(QScriptValueIterator);
    d->index = -1;
    d->nextIndex = -1;
}

/*!
  Moves the iterator to the back of the QScriptValue (after the
  last property).
*/
void QScriptValueIterator::toBack()
{
    Q_D(QScriptValueIterator);
    d->index = QScriptValuePrivate::valueOf(d->object).memberCount();
    d->nextIndex = -1;
}

/*!
  Returns the name of the last property that was jumped over using
  next() or previous().
*/
QString QScriptValueIterator::name() const
{
    Q_D(const QScriptValueIterator);
    if (d->index == -1)
        return QString();

    QScript::Member member;
    QScriptValuePrivate::valueOf(d->object).member(d->index, &member);

    if (member.isObjectProperty() || member.nameId())
        return member.nameId()->s;

    else if (member.isNativeProperty()) {
        QScriptEngine *eng = d->object.engine();
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
        return eng_p->toString(member.id());
    }

    return QString();
}

/*!
  Returns the value of the last property that was jumped over using
  next() or previous().
*/
QScriptValue QScriptValueIterator::value() const
{
    Q_D(const QScriptValueIterator);
    if (d->index == -1)
        return QScriptValue();

    QScript::Member member;
    QScriptValuePrivate::valueOf(d->object).member(d->index, &member);
    if (!member.isValid())
        return QScriptValue();

    QScriptValueImpl result;
    QScriptValuePrivate::valueOf(d->object).get(member, &result);
    return result;
}

/*!
  Sets the \a value of the last property that was jumped over using
  next() or previous().
*/
void QScriptValueIterator::setValue(const QScriptValue &value)
{
    Q_D(const QScriptValueIterator);
    if (d->index == -1)
        return;

    QScript::Member member;
    QScriptValuePrivate::valueOf(d->object).member(d->index, &member);
    if (!member.isValid())
        return;

    QScriptValuePrivate::valueOf(d->object).put(member, QScriptValuePrivate::valueOf(value));
}

/*!
  Returns the flags of the last property that was jumped over using
  next() or previous().
*/
QScriptValue::PropertyFlags QScriptValueIterator::flags() const
{
    Q_D(const QScriptValueIterator);
    if (d->index == -1)
        return 0;

    QScript::Member member;
    QScriptValuePrivate::valueOf(d->object).member(d->index, &member);
    if (!member.isValid())
        return 0;

    return QScriptValue::PropertyFlags(member.flags());
}

/*!
  Removes the last property that was jumped over using next()
  or previous().
*/
void QScriptValueIterator::remove()
{
    Q_D(const QScriptValueIterator);
    if (d->index == -1)
        return;

    QScript::Member member;
    QScriptValuePrivate::valueOf(d->object).member(d->index, &member);
    if (!member.isValid())
        return;

    QScriptValuePrivate::valueOf(d->object).removeMember(member);
}

/*!
  Makes the iterator operate on \a object. The iterator is set to be
  at the front of the sequence of properties (before the first
  property).
*/
QScriptValueIterator& QScriptValueIterator::operator=(QScriptValue &object)
{
    Q_D(QScriptValueIterator);
    d->object = object;
    d->index = -1;
    d->nextIndex = -1;
    return *this;
}
