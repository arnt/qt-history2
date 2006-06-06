/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QALGORITHMS_H
#define QALGORITHMS_H

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

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
template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator, BiIterator, const T &);

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBoundHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan);
template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBoundHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan);
template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFindHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan);

}

namespace std { struct bidirectional_iterator_tag; struct random_access_iterator_tag; }

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

template <typename Container, typename T>
inline void qFill(Container &container, const T &val)
{
    qFill(container.begin(), container.end(), val);
}

template <typename InputIterator, typename T>
inline InputIterator qFind(InputIterator first, InputIterator last, const T &val)
{
    while (first != last && !(*first == val))
        ++first;
    return first;
}

template <typename Container, typename T>
inline void qFind(const Container &container, const T &val)
{
    qFind(container.constBegin(), container.constEnd(), val);
}

template <typename InputIterator, typename T, typename Size>
inline void qCount(InputIterator first, InputIterator last, const T &value, Size &n)
{
    for (; first != last; ++first)
        if (*first == value)
            ++n;
}

template <typename Container, typename T, typename Size>
inline void qCount(const Container &container, const T &value, Size &n)
{
    qCount(container.constBegin(), container.constEnd(), value, n);
}

template <typename InputIterator, typename T>
inline int qCount(InputIterator first, InputIterator last, const T &value)
{
    int n = 0;
    for (; first != last; ++first)
        if (*first == value)
            ++n;
    return n;
}

template <typename Container, typename T>
inline int qCount(const Container &container, const T &value)
{
    return qCount(container.constBegin(), container.constEnd(), value);
}

template <typename T>
inline void qSwap(T &value1, T &value2)
{
    if (!QTypeInfo<T>::isComplex || QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic) {
        T t = value1;
        value1 = value2;
        value2 = t;
    } else {
        const void * const t = reinterpret_cast<const void * const &>(value1);
        const_cast<const void *&>(reinterpret_cast<const void * const &>(value1)) =
            reinterpret_cast<const void * const &>(value2);
        const_cast<const void *&>(reinterpret_cast<const void * const &>(value2)) = t;
    }
}

#ifdef qdoc
template <typename T>
LessThan qLess()
{
}

template <typename T>
LessThan qGreater()
{
}
#else
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
#endif

template <typename BiIterator>
inline void qSort(BiIterator start, BiIterator end)
{
    if (start != end)
        QAlgorithmsPrivate::qSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    if (start != end)
        QAlgorithmsPrivate::qSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    if (!c.empty())
        QAlgorithmsPrivate::qSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename BiIterator>
inline void qStableSort(BiIterator start, BiIterator end)
{
    if (start != end)
        QAlgorithmsPrivate::qStableSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qStableSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    if (start != end)
        QAlgorithmsPrivate::qStableSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qStableSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    if (!c.empty())
        QAlgorithmsPrivate::qStableSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    // Implementation is duplicated from QAlgorithmsPrivate to keep existing code
    // compiling. We have to allow using *begin and value with different types, 
    // and then implementing operator< for those types.
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

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    return QAlgorithmsPrivate::qLowerBoundHelper(begin, end, value, lessThan);
}

template <typename Container, typename T>
Q_OUTOFLINE_TEMPLATE typename Container::const_iterator qLowerBound(const Container &container, const T &value)
{
    return QAlgorithmsPrivate::qLowerBoundHelper(container.constBegin(), container.constEnd(), value, qLess<T>());
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    // Implementation is duplicated from QAlgorithmsPrivate.
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

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    return QAlgorithmsPrivate::qUpperBoundHelper(begin, end, value, lessThan);
}

template <typename Container, typename T>
Q_OUTOFLINE_TEMPLATE typename Container::const_iterator qUpperBound(const Container &container, const T &value)
{
    return QAlgorithmsPrivate::qUpperBoundHelper(container.constBegin(), container.constEnd(), value, qLess<T>());
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    // Implementation is duplicated from QAlgorithmsPrivate.
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

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    return QAlgorithmsPrivate::qBinaryFindHelper(begin, end, value, lessThan);
}

template <typename Container, typename T>
Q_OUTOFLINE_TEMPLATE typename Container::const_iterator qBinaryFind(const Container &container, const T &value)
{
    return QAlgorithmsPrivate::qBinaryFindHelper(container.constBegin(), container.constEnd(), value, qLess<T>());
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

template <typename T>
Q_OUTOFLINE_TEMPLATE std::random_access_iterator_tag *qIteratorCategory(T *)
{
    return 0;
};

template <typename Iterator>
Q_OUTOFLINE_TEMPLATE typename Iterator::iterator_category *qIteratorCategory(Iterator)
{
    return 0;
};

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortImplementation(RandomAccessIterator start, RandomAccessIterator end, const T &t, LessThan lessThan)
{
top:
    int span = end - start;
    if (span < 2)
        return;

    --end;
    RandomAccessIterator low = start, high = end - 1;
    RandomAccessIterator pivot = start + span / 2;

    if (lessThan(*end, *start))
        qSwap(*end, *start);
    if (span == 2)
        return;

    if (lessThan(*pivot, *start))
        qSwap(*pivot, *start);
    if (lessThan(*end, *pivot))
        qSwap(*end, *pivot);
    if (span == 3)
        return;

    qSwap(*pivot, *end);

    while (low < high) {
        while (low < high && lessThan(*low, *end))
            ++low;

        while (high > low && lessThan(*end, *high))
            --high;

        if (low < high) {
            qSwap(*low, *high);
            ++low;
            --high;
        } else {
            break;
        }
    }

    if (lessThan(*low, *end))
        ++low;

    qSwap(*end, *low);
    qSortImplementation(start, low, t, lessThan);

    start = low + 1;
    ++end;
    goto top;
}

/*
    Sort containers that only have bidirectional iterators by making a copy of the container
    and then sorting that.
*/
template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan, std::bidirectional_iterator_tag*)
{
    QList<T> tmp;
    BiIterator it = begin;
    while (it != end)
        tmp.append(*it++);

    qSortImplementation(tmp.begin(), tmp.end(), t, lessThan);
    qCopy(tmp.begin(), tmp.end(), begin);
}

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan, std::random_access_iterator_tag*)
{
    qSortImplementation(begin, end, t, lessThan);
}

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan)
{
    qSortHelper(begin, end, t, lessThan, qIteratorCategory(begin));
}

template <typename BiIterator, typename T>
inline void qSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qSortHelper(begin, end, dummy, qLess<T>());
}

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortImplementation(RandomAccessIterator start, RandomAccessIterator end, const T &t, LessThan lessThan)
{
    const int span = end - start;
    if (span < 2)
       return;
        
    // Split in half and sort halves.
    RandomAccessIterator middle = start + span / 2;
    qStableSortImplementation(start, middle, t, lessThan);
    qStableSortImplementation(middle, end, t, lessThan);
    
    // Merge
    RandomAccessIterator lo = start;
    RandomAccessIterator hi = middle;
    
    while (lo != middle && hi != end) {
        if (!lessThan(*hi, *lo)) {
            ++lo; // OK, *lo is in its correct position
        } else {
            // Move *hi to lo's position, shift values
            // between lo and hi - 1 one place up.
            T value = *hi;
            for (RandomAccessIterator i = hi; i != lo; --i) {
                *i = *(i-1); 
            }
            *lo = value;
            ++hi;
            ++lo;
            ++middle;
        } 
    }
}

/*
    Sort containers that only have bidirectional iterators by making a copy of the container
    and then sorting that.
*/
template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan, std::bidirectional_iterator_tag*)
{
    QList<T> tmp;
    BiIterator it = begin;
    while (it != end)
        tmp.append(*it++);

    qStableSortImplementation(tmp.begin(), tmp.end(), t, lessThan);
    qCopy(tmp.begin(), tmp.end(), begin);
}

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan, std::random_access_iterator_tag*)
{
    qStableSortImplementation(begin, end, t, lessThan);
}

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator begin, BiIterator end, const T &t, LessThan lessThan)
{
    qStableSortHelper(begin, end, t, lessThan, qIteratorCategory(begin));
}

template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qStableSortHelper(begin, end, dummy, qLess<T>());
}

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBoundHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (lessThan(*middle, value)) {
            begin = middle + 1;
            n -= half + 1;
        } else {
            n = half;
        }
    }
    return begin;
}


template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBoundHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (lessThan(value, *middle)) {
            n = half;
        } else {
            begin = middle + 1;
            n -= half + 1;
        }
    }
    return begin;
}

template <typename RandomAccessIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFindHelper(RandomAccessIterator begin, RandomAccessIterator end, const T &value, LessThan lessThan)
{
    int l = 0;
    int r = end - begin - 1;
    if (r < 0)
        return end;
    int i = (l + r + 1) / 2;

    while (r != l) {
        if (lessThan(value, begin[i]))
            r = i - 1;
        else
            l = i;
        i = (l + r + 1) / 2;
    }
    if (lessThan(begin[i], value) || lessThan(value, begin[i]))
        return end;
    else
        return begin + i;
}

} //namespace QAlgorithmsPrivate

QT_END_HEADER

#endif // QALGORITHMS_H
