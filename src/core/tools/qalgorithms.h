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

#include "qglobal.h"
#include "qstring.h"
#include "qtextstream.h"

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
bool qLess(const T &t1, const T &t2)
{ return t1 < t2; }

template <typename T>
bool qGreater(const T &t1, const T &t2)
{ return t1 > t2; }

template <typename BiIterator, typename LessThan>
void qBubbleSort(BiIterator begin, BiIterator end, LessThan lessThan)
{
    // Goto last element;
    BiIterator last = end;
    --last;
    // only one element or no elements ?
    if (last == begin)
        return;

    // So we have at least two elements in here
    while (begin != last) {
        bool swapped = false;
        BiIterator swapPos = begin;
        BiIterator x = end;
        BiIterator y = x;
        y--;
        do {
            --x;
            --y;
            if (lessThan(*x, *y)) {
                swapped = true;
                qSwap(*x, *y);
                swapPos = y;
            }
        } while (y != begin);
        if (!swapped)
            return;
        begin = swapPos;
        ++begin;
    }
}

template <typename BiIterator, typename T>
void qBubbleSortHelper(BiIterator begin, BiIterator end, T)
{
    qBubbleSort(begin, end, qLess<T>);
}

template <typename BiIterator>
void qBubbleSort(BiIterator begin, BiIterator end)
{
    qBubbleSortHelper(begin, end, *begin);
}

template <typename Container>
inline void qBubbleSort(Container &c)
{
    qBubbleSort(c.begin(), c.end());
}

template <typename T, typename LessThan>
void qHeapSortPushDown(T *heap, int first, int last, LessThan lessThan)
{
    int r = first;
    while (r <= last / 2) {
        if (last == 2 * r) {
            // node r has only one child
            if (lessThan(heap[2 * r], heap[r]))
                qSwap(heap[r], heap[2 * r]);
            r = last;
        } else {
            // node r has two children
            if (lessThan(heap[2 * r], heap[r]) && !lessThan(heap[2 * r + 1], heap[2 * r])) {
                // swap with left child
                qSwap(heap[r], heap[2 * r]);
                r *= 2;
            } else if (lessThan(heap[2 * r + 1], heap[r])
                       && lessThan(heap[2 * r + 1], heap[2 * r])) {
                // swap with right child
                qSwap(heap[r], heap[2 * r + 1]);
                r = 2 * r + 1;
            } else {
                r = last;
            }
        }
    }
}

template <typename BiIterator, typename T, typename LessThan>
void qHeapSortHelper(BiIterator begin, BiIterator end, const T & /* dummy */, LessThan lessThan)
{
    BiIterator it = begin;
    uint n = 0;
    while (it != end) {
        ++n;
        ++it;
    }
    if (n == 0)
        return;

    // Create the heap
    BiIterator insert = begin;
    T *realheap = new T[n];
    T *heap = realheap - 1;
    int size = 0;
    for(; insert != end; ++insert) {
        heap[++size] = *insert;
        int i = size;
        while (i > 1 && lessThan(heap[i], heap[i / 2])) {
            qSwap(heap[i], heap[i / 2]);
            i /= 2;
        }
    }

    // Now do the sorting
    for (int i = n; i > 0; i--) {
        *begin++ = heap[1];
        if (i > 1) {
            heap[1] = heap[i];
            qHeapSortPushDown(heap, 1, i - 1, lessThan);
        }
    }

    delete[] realheap;
}

template <typename BiIterator, typename T>
void qHeapSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    // Don't pass qLess<T> directly (workaround for MSVC)
    bool (*qLessFunc)(const T &a, const T &b) = qLess<T>;
    qHeapSortHelper(begin, end, dummy, qLessFunc);
}

template <typename BiIterator, typename LessThan>
void qHeapSort(BiIterator begin, BiIterator end, LessThan lessThan)
{
    qHeapSortHelper(begin, end, *begin, lessThan);
}

template <typename BiIterator>
void qHeapSort(BiIterator begin, BiIterator end)
{
    qHeapSortHelper(begin, end, *begin);
}

template <typename Container>
void qHeapSort(Container &c)
{
    qHeapSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename RandomAccessIterator, typename T>
RandomAccessIterator qLowerBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
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
RandomAccessIterator qUpperBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
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
RandomAccessIterator qBinaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    int l = 0;
    int r = end - begin - 1;
    if (r <= 0)
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
void qDeleteAll(ForwardIterator begin, ForwardIterator end)
{
    while (begin != end) {
        delete *begin;
        ++begin;
    }
}

template <typename Container>
void qDeleteAll(const Container &c)
{
    qDeleteAll(c.begin(), c.end());
}
#endif
