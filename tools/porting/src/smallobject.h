/****************************************************************************
**
** Copyright (C) 2001-2004 Roberto Raggi
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SMALLOBJECT_H
#define SMALLOBJECT_H

#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <QList>

struct block_t
{
    static int N;

    block_t *chain;
    char *data;
    char *ptr;
    char *end;

    inline void init(int block_size=256)
    {
        ++N;
        chain = 0;
        data = (char*) malloc(block_size);
        ptr = data;
        end = data + block_size;
    }

    inline void init0(int block_size)
    {
        init();
        memset(data, '0', block_size);
    }

    inline void destroy()
    {
        --N;
        if (chain) {
            chain->destroy();
            free(chain);
        }

        free(data);
    }

    inline void *allocate(size_t size, block_t **right_most)
    {
        if (end < ptr + size) {
//            assert( size < block_size );

            if (!chain) {
                chain = (block_t*) malloc(sizeof(block_t));
                chain->init(1 << (8 + N));
            }

            return chain->allocate(size, right_most);
        }

        char *r = ptr;
        ptr += size;

        if (right_most)
            *right_most = this;

        return r;
    }

};

// a stupid memory pool
struct pool
{
    block_t blk;
    block_t *right_most;

    inline pool() { blk.init(); right_most = &blk; }
    inline ~pool() { blk.destroy(); }

    inline void *allocate(size_t size)
    {
//         return ::malloc(size);
        return right_most->allocate(size, &right_most);
    }
    inline void *reallocate(void *old, size_t old_size, size_t size)
    {
//         return ::realloc(old, size);
        void *alloc = right_most->allocate(size, &right_most);
        memcpy(alloc, old, old_size);
        return alloc;
    }
};

/*
    A memory pool that calls the destructor for each stored item when memory is
    freed, at the expence storing one BaseType* per item.

    Note that allocations for types that does not inherit BaseType
    is not allowed and will result in a crash when the memory is freed.
*/
template <typename BaseType>
class TypedPool
{
public:
    inline TypedPool() { blk.init(); right_most = &blk; }
    inline ~TypedPool()
    {
        foreach(BaseType *item, allocated)
            item->~BaseType();
        blk.destroy();
    }

    inline void *allocate(size_t size)
    {
        void *memory = right_most->allocate(size, &right_most);
        allocated.append(reinterpret_cast<BaseType *>(memory));
        return memory;
        Q_ASSERT(memory);
    }
private:
    block_t blk;
    block_t *right_most;
    QList<BaseType *> allocated;
};

struct SmallObject
{
    static void *operator new(size_t size);
    static void operator delete(void *p);
};

#endif
