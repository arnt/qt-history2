#ifndef QSTACK_H
#define QSTACK_H

#ifndef QT_H
#include "qvector.h"
#endif // QT_H

template<class T>
class QStack : public QVector<T>
{
public:
    inline QStack() {}
    inline ~QStack() {}
    inline void push(const T &t) { append(t); }
    T pop();
    T &top();
    const T &top() const;
};

template<class T>
inline T QStack<T>::pop()
{ Q_ASSERT(!this->isEmpty()) T t = this->data()[this->size() -1];
  this->resize(this->size()-1); return t; }

template<class T>
inline T &QStack<T>::top()
{ Q_ASSERT(!this->isEmpty()) this->detach(); return this->data()[this->size()-1]; }

template<class T>
inline const T &QStack<T>::top() const
{ Q_ASSERT(!this->isEmpty()) return this->data()[this->size()-1]; }

#endif // QSTACK_H
