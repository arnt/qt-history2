/****************************************************************************
**
** Definition of Qt template library classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTL_H
#define QTL_H

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qtextstream.h"
#endif // QT_H

#ifndef QT_NO_TEXTSTREAM
template <class T>
class QTextOStreamIterator
{
protected:
    QTextOStream &stream;
    QString separator;

public:
    QTextOStreamIterator(QTextOStream &s) : stream(s) {}
    QTextOStreamIterator(QTextOStream &s, const QString &sep)
	: stream(s), separator(sep)  {}
    QTextOStreamIterator<T> &operator= (const T &x) {
	stream << x;
	if (!separator.isEmpty())
	    stream << separator;
	return *this;
    }
    QTextOStreamIterator<T> &operator*() { return *this; }
    QTextOStreamIterator<T> &operator++() { return *this; }
    QTextOStreamIterator<T> &operator++(int) { return *this; }
};
#endif //QT_NO_TEXTSTREAM

template <class InputIterator, class OutputIterator>
inline OutputIterator qCopy(InputIterator _begin, InputIterator _end,
			    OutputIterator _dest)
{
    while(_begin != _end)
	*_dest++ = *_begin++;
    return _dest;
}

template <class BiIterator1, class BiIterator2>
inline BiIterator2 qCopyBackward(BiIterator1 _begin, BiIterator1 _end, BiIterator2 _dest)
{
    while (_begin != _end)
	*--_dest = *--_end;
    return _dest;
}

template <class InputIterator1, class InputIterator2>
inline bool qEqual(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
	if (!(*first1 == *first2))
	    return false;
    return true;
}

template <class ForwardIterator, class T>
inline void qFill(ForwardIterator first, ForwardIterator last, const T &val)
{
    for (; first != last; ++first)
	*first = val;
}

#if 0
template <class BiIterator, class OutputIterator>
inline OutputIterator qReverseCopy(BiIterator _begin, BiIterator _end, OutputIterator _dest)
{
    while (_begin != _end) {
	--_end;
	*_dest = *_end;
	++_dest;
    }
    return _dest;
}
#endif

template <class InputIterator, class T>
inline InputIterator qFind(InputIterator first, InputIterator last, const T &val)
{
    while (first != last && *first != val)
	++first;
    return first;
}

template <class InputIterator, class T, class Size>
inline void qCount(InputIterator first, InputIterator last, const T &value, Size &n)
{
    for (; first != last; ++first)
	if (*first == value)
	    ++n;
}

template <class T>
inline void qSwap(T &_value1, T &_value2)
{
    T tmp = _value1;
    _value1 = _value2;
    _value2 = tmp;
}

template <class BiIterator>
void qBubbleSort(BiIterator b, BiIterator e)
{
    // Goto last element;
    BiIterator last = e;
    --last;
    // only one element or no elements ?
    if (last == b)
	return;

    // So we have at least two elements in here
    while(b != last) {
	bool swapped = false;
	BiIterator swap_pos = b;
	BiIterator x = e;
	BiIterator y = x;
	y--;
	do {
	    --x;
	    --y;
	    if (*x < *y) {
		swapped = true;
		qSwap(*x, *y);
		swap_pos = y;
	    }
	} while(y != b);
	if (!swapped)
	    return;
	b = swap_pos;
	b++;
    }
}

template <class Container>
inline void qBubbleSort(Container &c)
{
    qBubbleSort(c.begin(), c.end());
}

template <class Value>
void qHeapSortPushDown(Value *heap, int first, int last)
{
    int r = first;
    while (r <= last / 2) {
	if (last == 2 * r) {
	    // node r has only one child
	    if (heap[2 * r] < heap[r])
		qSwap(heap[r], heap[2 * r]);
	    r = last;
	} else {
	    // node r has two children
	    if (heap[2 * r] < heap[r] && !(heap[2 * r + 1] < heap[2 * r])) {
		// swap with left child
		qSwap(heap[r], heap[2 * r]);
		r *= 2;
	    } else if (heap[2 * r + 1] < heap[r]
			&& heap[2 * r + 1] < heap[2 * r]) {
		// swap with right child
		qSwap(heap[r], heap[2 * r + 1]);
		r = 2 * r + 1;
	    } else {
		r = last;
	    }
	}
    }
}

template <class BiIterator, class Value>
void qHeapSortHelper(BiIterator b, BiIterator e, Value, uint n)
{
    // Create the heap
    BiIterator insert = b;
    Value *realheap = new Value[n];
    Value *heap = realheap - 1;
    int size = 0;
    for(; insert != e; ++insert) {
	heap[++size] = *insert;
	int i = size;
	while (i > 1 && heap[i] < heap[i / 2]) {
	    qSwap(heap[i], heap[i / 2]);
	    i /= 2;
	}
    }

    // Now do the sorting
    for (uint i = n; i > 0; i--) {
	*b++ = heap[1];
	if (i > 1) {
	    heap[1] = heap[i];
	    qHeapSortPushDown(heap, 1, (int)i - 1);
	}
    }

    delete[] realheap;
}

template <class BiIterator>
void qHeapSort(BiIterator b, BiIterator e)
{
    // Empty?
    if (b == e)
	return;

    // How many entries have to be sorted?
    BiIterator it = b;
    uint n = 0;
    while (it != e) {
	++n;
	++it;
    }

    // The second last parameter is a hack to retrieve the value type
    // Do the real sorting here
    qHeapSortHelper(b, e, *b, n);
}

template <class Container>
void qHeapSort(Container &c)
{
    if (c.begin() == c.end())
	return;

    // The second last parameter is a hack to retrieve the value type
    // Do the real sorting here
    qHeapSortHelper(c.begin(), c.end(), *c.begin(), (uint)c.count());
}

template <class RandomAccessIterator, class T>
RandomAccessIterator qLowerBound(RandomAccessIterator b, RandomAccessIterator e, const T &value)
{
    RandomAccessIterator middle;
    int n = e - b;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = b + half;
        if (*middle < value) {
            b = middle + 1;
            n -= half + 1;
        } else {
	    n = half;
	}
    }
    return b;
}

template <class Container, class T>
typename Container::const_iterator qLowerBound(const Container &c, const T &value)
{
    return qLowerBound(c.begin(), c.end(), value);
}

template <class RandomAccessIterator, class T>
RandomAccessIterator qUpperBound(RandomAccessIterator b, RandomAccessIterator e, const T &value)
{
    RandomAccessIterator middle;
    int n = e - b;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = b + half;
        if (value < *middle) {
	    n = half;
	} else {
            b = middle + 1;
            n -= half + 1;
        }
    }
    return b;
}

template <class Container, class T>
typename Container::const_iterator qUpperBound(const Container &c, const T &value)
{
    return qUpperBound(c.begin(), c.end(), value);
}

template <class RandomAccessIterator, class T>
bool qBinarySearch(RandomAccessIterator b, RandomAccessIterator e, const T &value)
{
    int l = 0;
    int r = e - b - 1;
    if (r <= 0)
	return false;
    int i = (l + r + 1) / 2;
    while (r != l) {
	if (value < b[i])
	    r = i - 1;
	else
	    l = i;
	i = (l + r + 1) / 2;
    }
    return !(b[i] < value) && !(value < b[i]);
}

template <class Container, class T>
bool qBinarySearch(const Container &c, const T &value)
{
    return qBinarySearch(c.begin(), c.end(), value);
}

template <class ForwardIterator>
void qDeleteAll(ForwardIterator b, ForwardIterator e)
{
    while (b != e) {
	delete *b;
        ++b;
    }
}

template <class Container>
void qDeleteAll(const Container &c)
{
    qDeleteAll(c.begin(), c.end());
}

template <class Container>
class QBackInsertIterator
{
public:
    inline explicit QBackInsertIterator(Container &c): container(&c) {}
    inline QBackInsertIterator<Container>&operator=(const typename Container::value_type &value)
    { container->push_back(value); return *this; }
    inline QBackInsertIterator<Container> &operator*() { return *this; }
    inline QBackInsertIterator<Container> &operator++() { return *this; }
    inline QBackInsertIterator<Container> &operator++(int) { return *this; }
protected:
    Container *container;
};

template <class Container>
inline QBackInsertIterator<Container> qBackInserter(Container &c)
{ return QBackInsertIterator<Container>(c); }

#endif
