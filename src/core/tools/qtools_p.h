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

inline int qAllocMore(int alloc, int extra)
#ifndef QT_H
#endif // QT_H
{
    const int page = 1<<12;
    int nalloc;
    alloc += extra;
    if (alloc < 1<<6) {
        nalloc = (1<<3) + ((alloc >>3) << 3);
    } else if (alloc < page) {
        nalloc = 1<<3;
        while (nalloc < alloc)
            nalloc *= 2;
    } else {
        nalloc = ((alloc + page) / page) * page;
    }
    return nalloc - extra;
}


