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

#ifndef QSCRIPTARRAY_P_H
#define QSCRIPTARRAY_P_H

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

#include <QtCore/QMap>
#include <QtCore/QVector>

#include "qscriptvalue.h"

namespace QScript {

class Array // ### private
{
public:
    inline Array();
    inline Array(const Array &other);
    inline ~Array();

    inline Array &operator = (const Array &other);

    inline bool isEmpty() const;
    inline uint size() const;
    inline uint count() const;
    inline QScriptValue at(uint index) const;
    inline void assign(uint index, const QScriptValue &v);
    inline void clear();
    inline void mark(int generation);
    inline void resize(uint size);
    inline void concat(const Array &other);
    inline QScriptValue pop();
    inline void sort(const QScriptValue &comparefn);
    inline void splice(qnumber start, qnumber deleteCount,
                       const QVector<QScriptValue> &items,
                       Array &other);

private:
    enum Mode {
        VectorMode,
        MapMode,
    };

    Mode m_mode;
    int m_instances;

    union {
        QMap<uint, QScriptValue> *to_map;
        QVector<QScriptValue> *to_vector;
    };
};

class ArrayElementLessThan
{
public:
    inline ArrayElementLessThan(const QScriptValue &comparefn)
        : m_comparefn(comparefn) {}

    inline bool operator()(const QScriptValue &v1, const QScriptValue &v2) const
    {
        if (v1.isUndefined())
            return false;
        if (v2.isUndefined())
            return true;
        if (!m_comparefn.isUndefined()) {
            ArrayElementLessThan *that = const_cast<ArrayElementLessThan*>(this);
            QScriptValue result = that->m_comparefn.call(QScriptValue(),
                                                         QScriptValueList() << v1 << v2);
            return result.toNumber() <= 0;
        }
        return v1.toString() < v2.toString();
    }

private:
    QScriptValue m_comparefn;
};

} // namespace QScript

inline QScript::Array::Array():
    m_mode(VectorMode),
    m_instances(0)
{
    to_vector = new QVector<QScriptValue>();
}

inline QScript::Array::Array(const Array &other):
    m_mode(other.m_mode),
    m_instances(other.m_instances)
{
    if (m_mode == VectorMode)
        to_vector = new QVector<QScriptValue> (*other.to_vector);
    else
        to_map = new QMap<uint, QScriptValue> (*other.to_map);
}

inline QScript::Array::~Array()
{
    if (m_mode == VectorMode)
        delete to_vector;
    else
        delete to_map;
}

inline QScript::Array &QScript::Array::operator = (const Array &other)
{
    if (m_mode != other.m_mode) {
        if (m_mode == VectorMode)
            delete to_vector;
        else
            delete to_map;
        m_mode = other.m_mode;
        m_instances = other.m_instances;

        if (m_mode == VectorMode)
            to_vector = new QVector<QScriptValue> (*other.to_vector);
        else
            to_map = new QMap<uint, QScriptValue> (*other.to_map);
    }

    if (m_mode == VectorMode)
        *to_vector = *other.to_vector;
    else
        *to_map = *other.to_map;

    return *this;
}

inline bool QScript::Array::isEmpty() const
{
    if (m_mode == VectorMode)
        return to_vector->isEmpty();

    return to_map->isEmpty();
}

inline uint QScript::Array::size() const
{
    if (m_mode == VectorMode)
        return to_vector->size();

    if (to_map->isEmpty())
        return 0;

    return (--to_map->end()).key();
}

inline uint QScript::Array::count() const
{
    return size();
}

inline QScriptValue QScript::Array::at(uint index) const
{
    QScriptValue v;

    if (m_mode == VectorMode) {
        if (index < uint(to_vector->size()))
            v = to_vector->at(index);
        else
            v.invalidate();
    }

    else
        v = to_map->value(index, QScriptValue());

    return v;
}

inline void QScript::Array::assign(uint index, const QScriptValue &v)
{
    if (index >= size())
        resize(index + 1);

    if (v.isValid() && (v.isObject() || v.isString()))
        ++m_instances;

    if (m_mode == VectorMode)
        to_vector->replace(index, v);

    else
        to_map->insert(index, v);
}

inline void QScript::Array::clear()
{
    m_instances = 0;
    
    if (m_mode == VectorMode)
        to_vector->clear();

    else
        to_map->clear();
}

inline void QScript::Array::mark(int generation)
{
    if (! m_instances)
        return;
        
    if (m_mode == VectorMode) {
        for (int i = 0; i < to_vector->size(); ++i)
            to_vector->at(i).mark(generation);
    } else {
        QMap<uint, QScriptValue>::const_iterator it = to_map->constBegin();
        for (; it != to_map->constEnd(); ++it)
            it.value().mark(generation);
    }
}

inline void QScript::Array::resize(uint s)
{
    const uint N = 10 * 1024;

    if (m_mode == VectorMode) {
        if (s < N) {
            int oldSize = to_vector->size();
            to_vector->resize (s);
            if (oldSize < to_vector->size()) {
                for (int i = oldSize; i < to_vector->size(); ++i)
                    (*to_vector)[i].invalidate();
            }
        }

        else {
            QMap<uint, QScriptValue> *m = new QMap<uint, QScriptValue>();
            for (uint i = 0; i < uint(to_vector->size()); ++i)
                m->insert(i, to_vector->at(i));
            m->insert(s, QScriptValue());
            delete to_vector;
            to_map = m;
            m_mode = MapMode;
        }
    }

    else {
        if (s < N) {
            QVector<QScriptValue> *v = new QVector<QScriptValue> ();
            v->fill(QScriptValue(), s);
            QMap<uint, QScriptValue>::const_iterator it = to_map->constBegin();
            uint i = 0;
            for (; i < s && it != to_map->constEnd(); ++it, ++i)
                (*v) [i] = it.value();
            delete to_map;
            to_vector = v;
            m_mode = VectorMode;
            return;
        }

        if (! to_map->isEmpty()) {
            QMap<uint, QScriptValue>::iterator it = to_map->insert(s, QScriptValue());
            for (++it; it != to_map->end(); )
                it = to_map->erase(it);
            to_map->insert(s, QScriptValue()); // ### hmm
        }
    }
}

inline void QScript::Array::concat(const QScript::Array &other)
{
    int k = size();
    resize (k + other.size());
    QScriptValue def;
    def.invalidate();
    for (uint i = 0; i < other.size(); ++i) {
        QScriptValue v = other.at(i);
        if (! v.isValid())
            continue;

        assign(k + i, v);
    }
}

inline QScriptValue QScript::Array::pop()
{
    if (isEmpty())
        return QScriptValue();

    QScriptValue v;

    if (m_mode == VectorMode)
        v = to_vector->last();

    else
        v = *--to_map->end();

    resize(size() - 1);

    return v;
}

inline void QScript::Array::sort(const QScriptValue &comparefn)
{
    ArrayElementLessThan lessThan(comparefn);
    if (m_mode == VectorMode) {
        qSort(to_vector->begin(), to_vector->end(), lessThan);
    } else {
        QList<uint> keys = to_map->keys();
        QList<QScriptValue> values = to_map->values();
        qSort(values.begin(), values.end(), lessThan);
        const uint len = size();
        for (uint i = 0; i < len; ++i)
            to_map->insert(keys.at(i), values.at(i));
    }
}

inline void QScript::Array::splice(qnumber start, qnumber deleteCount,
                                   const QVector<QScriptValue> &items,
                                   Array &other)
{
    const qnumber len = size();
    if (start < 0)
        start = qMax(len + start, qnumber(0));
    else if (start > len)
        start = len;
    deleteCount = qMax(qMin(deleteCount, len), qnumber(0));

    const uint st = uint(start);
    const uint dc = uint(deleteCount);
    other.resize(dc);

    if (m_mode == VectorMode) {

        for (uint i = 0; i < dc; ++i) {
            QScriptValue v = to_vector->at(st + i);
            other.assign(i, v);
            if (i < uint(items.size()))
                to_vector->replace(st + i, items.at(i));
        }
        to_vector->remove(st + items.size(), dc - items.size());

    } else {

        for (uint i = 0; i < dc; ++i) {
            QScriptValue v = to_map->value(st + i, QScriptValue());
            other.assign(i, v);
            if (i < uint(items.size()))
                to_map->insert(st + i, items.at(i));
        }

        uint del = dc - items.size();
        if (del != 0) {
            for (uint j = st + items.size(); j < uint(len); ++j) {
                if (to_map->contains(j)) {
                    QScriptValue v = to_map->take(j);
                    to_map->insert(j - del, v);
                }
            }
            resize(uint(len) - del);
        }

    }
}

#endif // QSCRIPTARRAY_P_H
