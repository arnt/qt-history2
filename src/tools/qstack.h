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
{ Q_ASSERT(!isEmpty()) T t = data() [size() -1];
  resize(size()-1); return t; }
template<class T>
inline T &QStack<T>::top()
{ Q_ASSERT(!isEmpty()) return detach()[size()-1]; }
template<class T>
inline const T &QStack<T>::top() const
{ Q_ASSERT(!isEmpty()) return data()[size()-1]; }

#endif // QSTACK_H
