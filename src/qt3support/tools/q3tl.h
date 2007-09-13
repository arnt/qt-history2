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

#ifndef Q3TL_H
#define Q3TL_H

#include <QtCore/qalgorithms.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

template <typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qHeapSortPushDown(T *heap, int first, int last, LessThan lessThan)
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
Q_OUTOFLINE_TEMPLATE void qHeapSortHelper(BiIterator begin, BiIterator end, const T & /* dummy */, LessThan lessThan)
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
inline void qHeapSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qHeapSortHelper(begin, end, dummy, qLess<T>());
}

template <typename BiIterator, typename LessThan>
inline void qHeapSort(BiIterator begin, BiIterator end, LessThan lessThan)
{
    if (begin != end)
        qHeapSortHelper(begin, end, *begin, lessThan);
}

template <typename BiIterator>
inline void qHeapSort(BiIterator begin, BiIterator end)
{
    if (begin != end)
        qHeapSortHelper(begin, end, *begin);
}

template <typename Container>
inline void qHeapSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    if (!c.empty())
        qHeapSortHelper(c.begin(), c.end(), *c.begin());
}


template <typename BiIterator, typename LessThan>
void qBubbleSort(BiIterator begin, BiIterator end, LessThan lessThan)
{
    // Goto last element;
    BiIterator last = end;

    // empty list
    if (begin == end)
        return;

    --last;
    // only one element ?
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
    qBubbleSort(begin, end, qLess<T>());
}

template <typename BiIterator>
void qBubbleSort(BiIterator begin, BiIterator end)
{
    if (begin != end)
        qBubbleSortHelper(begin, end, *begin);
}

template <typename Container>
inline void qBubbleSort(Container &c)
{
    qBubbleSort(c.begin(), c.end());
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3TL_H
