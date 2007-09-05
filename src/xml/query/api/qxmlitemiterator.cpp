/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QVariant>

#include "CommonValues.h"

#include "qxmlitemiterator_p.h"
#include "qxmlitemiterator.h"

/*!
  \class QXmlItemIterator
  \brief
  \reentrant
  \since 4.4

 */

/*!
  Constructs an empty QXmlItemIterator.
 */
QXmlItemIterator::QXmlItemIterator() : d(new QXmlItemIteratorPrivate(Patternist::DynamicContext::Ptr(), Patternist::Expression::Ptr()))
{
}

/*!
  Constructs an QXmlItemIterator instance that is a copy of \a other.
 */
QXmlItemIterator::QXmlItemIterator(const QXmlItemIterator &other) : d(new QXmlItemIteratorPrivate(*other.d))
{
}

/*!
 \internal
 */
QXmlItemIterator::QXmlItemIterator(QXmlItemIteratorPrivate *const p) : d(p)
{
}

/*!
  Destructs this QXmlItemIterator instance.
 */
QXmlItemIterator::~QXmlItemIterator()
{
    delete d;
}

/*!
  Assigns \a other to this QXmlItemIterator instance.
 */
QXmlItemIterator &QXmlItemIterator::operator=(const QXmlItemIterator &other)
{
    if(this != &other)
    {
        // TODO
        Q_ASSERT(false);
    }

    return *this;
}

void QXmlItemIterator::next()
{
    d->current = d->next;

    try
    {
        d->next = d->it->next();
    }
    catch(const Patternist::Exception &)
    {
        d->next = Patternist::Item();
    }
}

bool QXmlItemIterator::isAtomicValue() const
{
    return d->current.isAtomicValue();
}

bool QXmlItemIterator::isNode() const
{
    return d->current.isNode();
}

QVariant QXmlItemIterator::atomicValue() const
{
    return Patternist::AtomicValue::toQt(d->current.asAtomicValue());
}

const void *QXmlItemIterator::toNodePointer() const
{
    return d->current.asNode().internalPointer();
}

bool QXmlItemIterator::hasNext() const
{
    return d->next;

}

// vim: et:ts=4:sw=4:sts=4
