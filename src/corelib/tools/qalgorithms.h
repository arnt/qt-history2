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

#ifndef QALGORITHMS_H
#define QALGORITHMS_H

#include "QtCore/qglobal.h"

/*
    Warning: The contents of QAlgorithmsPrivate is not a part of the public Qt API
    and may be changed from version to version or even be completely removed.
*/
namespace QAlgorithmsPrivate {

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan);
template <typename BiIterator, typename T>
inline void qSortHelper(BiIterator begin, BiIterator end, const T &dummy);

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan);
template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper2(BiIterator, BiIterator, LessThan, T *);
template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator, BiIterator, const T &);

}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator qCopy(InputIterator begin, InputIterator end, OutputIterator dest)
{
    while (begin != end)
        *dest++ = *begin++;
    return dest;
}

template <typename BiIterator1, typename BiIterator2>
inline BiIterator2 qCopyBackward(BiIterator1 begin, BiIterator1 end, BiIterator2 dest)
{
    while (begin != end)
        *--dest = *--end;
    return dest;
}

template <typename InputIterator1, typename InputIterator2>
inline bool qEqual(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        if (!(*first1 == *first2))
            return false;
    return true;
}

template <typename ForwardIterator, typename T>
inline void qFill(ForwardIterator first, ForwardIterator last, const T &val)
{
    for (; first != last; ++first)
        *first = val;
}

template <typename InputIterator, typename T>
inline InputIterator qFind(InputIterator first, InputIterator last, const T &val)
{
    while (first != last && !(*first == val))
        ++first;
    return first;
}

template <typename InputIterator, typename T, typename Size>
inline void qCount(InputIterator first, InputIterator last, const T &value, Size &n)
{
    for (; first != last; ++first)
        if (*first == value)
            ++n;
}

template <typename T>
inline void qSwap(T &value1, T &value2)
{
    T tmp = value1;
    value1 = value2;
    value2 = tmp;
}

template <typename T>
class qLess
{
public:
    inline bool operator()(const T &t1, const T &t2) const
    {
        return (t1 < t2);
    }
};

template <typename T>
class qGreater
{
public:
    inline bool operator()(const T &t1, const T &t2) const
    {
        return (t2 < t1);
    }
};

template <typename BiIterator>
inline void qSort(BiIterator start, BiIterator end)
{
    QAlgorithmsPrivate::qSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    QAlgorithmsPrivate::qSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    QAlgorithmsPrivate::qSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename BiIterator>
inline void qStableSort(BiIterator start, BiIterator end)
{
    QAlgorithmsPrivate::qStableSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qStableSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    QAlgorithmsPrivate::qStableSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qStableSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    QAlgorithmsPrivate::qStableSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (*middle < value) {
            begin = middle + 1;
            n -= half + 1;
        } else {
            n = half;
        }
    }
    return begin;
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (value < *middle) {
            n = half;
        } else {
            begin = middle + 1;
            n -= half + 1;
        }
    }
    return begin;
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    int l = 0;
    int r = end - begin - 1;
    if (r < 0)
        return end;
    int i = (l + r + 1) / 2;

    while (r != l) {
        if (value < begin[i])
            r = i - 1;
        else
            l = i;
        i = (l + r + 1) / 2;
    }
    if (begin[i] < value || value < begin[i])
        return end;
    else
        return begin + i;
}

template <typename ForwardIterator>
Q_OUTOFLINE_TEMPLATE void qDeleteAll(ForwardIterator begin, ForwardIterator end)
{
    while (begin != end) {
        delete *begin;
        ++begin;
    }
}

template <typename Container>
inline void qDeleteAll(const Container &c)
{
    qDeleteAll(c.begin(), c.end());
}

/*
    Warning: The contents of QAlgorithmsPrivate is not a part of the public Qt API
    and may be changed from version to version or even be completely removed.
*/
namespace QAlgorithmsPrivate {

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan)
{
    --end;
    if (end - start < 1)
        return;

    BiIterator pivot = start + (end - start) / 2;

    T pivot_val = *pivot;
    *pivot = *end;
    *end = pivot_val;

    BiIterator low = start, high = end-1;

    while (low < high) {
        while (low < high && lessThan(*low, pivot_val))
            ++low;

        while (high > low && lessThan(pivot_val, *high))
            --high;

        if(low < high) {
            T tmp = *low;
            *low = *high;
            *high = tmp;
            ++low;
            --high;
        }
    }

    if (lessThan(*low, pivot_val))
        ++low;

    *end = *low;
    *low = pivot_val;

    qSortHelper(start, low, t, lessThan);
    qSortHelper(low + 1, end + 1, t, lessThan);
}

template <typename BiIterator, typename T>
inline void qSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qSortHelper(begin, end, dummy, qLess<T>());
}


template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper2(BiIterator start, BiIterator end, LessThan lessThan, T * buf)
{
    if(end - start < 2)
       return;

    BiIterator middle = start + (end - start) / 2;
    qStableSortHelper2(start, middle, lessThan, buf);
    qStableSortHelper2(middle, end, lessThan, buf);

    BiIterator pos = start;
    T *bufPos = buf;
    while(pos != (end)) {
        *bufPos++ = *pos++;
    };

    T *bufEnd = bufPos;
    T *bufMiddle = buf  + (bufEnd - buf) /2;
    T *b1 = buf;
    T *b2 = bufMiddle;

    pos = start;
    while(b1 < bufMiddle && b2 < bufEnd) {
        if(lessThan(*b2, *b1))
            *pos++ = *b2++;
        else
            *pos++ = *b1++;
    }

    while(b1 < bufMiddle )
        *pos++ = *b1++;
    while(b2 < bufEnd)
        *pos++ = *b2++;
}


template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan)
{
    Q_UNUSED(t);
    if(end - start < 2)
       return;

    int size = end - start;
    T *buf = new T[size];
    qStableSortHelper2(start, end, lessThan, buf);
    delete[] buf;
}


template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qStableSortHelper(begin, end, dummy, qLess<T>());
}

} //namespace QAlgorithmsPrivate


#endif // QALGORITHMS_H
