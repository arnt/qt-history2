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

#ifndef QSCRIPTGC_P_H
#define QSCRIPTGC_P_H

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

#include <QtCore/qglobal.h>
#include <QtCore/QtDebug>
#include <new>

#include "qscriptmemorypool_p.h"

namespace QScript {

class GCBlock
{
public:
    GCBlock *next;

    union {
        int generation;
        uint flags;
    };

public:
    inline GCBlock(GCBlock *n):
        next(n), flags(0) {}

    inline void *data()
    { return reinterpret_cast<char *>(this) + sizeof(GCBlock); }

    inline static GCBlock *get(void *ptr)
    {
        char *where = reinterpret_cast<char *>(ptr);
        return reinterpret_cast<GCBlock *>(where - sizeof(GCBlock));
    }
};

template <typename _Tp>
class GCAlloc
{
private:
    int m_new_allocated_blocks;
    int m_free_blocks;
    GCBlock *m_head;
    GCBlock *m_current;
    GCBlock *m_free;
    bool m_blocked_gc;
    bool m_force_gc;
    MemoryPool pool;
    _Tp trivial;

public:
    enum { MaxNumberOfBlocks = 1 << 14 };

public:
    inline GCAlloc():
        m_new_allocated_blocks(0),
        m_free_blocks(0),
        m_head(0),
        m_current(0),
        m_free(0),
        m_blocked_gc(false),
        m_force_gc(false) {}

    inline ~GCAlloc() {
    }

    inline void destruct() {
        GCBlock *blk = m_free;

        if (! blk) {
            blk = m_head;
            m_head = 0;
        }

        while (blk) {
            GCBlock *was = blk;
            blk = blk->next;

            Q_ASSERT(was->data());
            _Tp *data = reinterpret_cast<_Tp*>(was->data());
            data->finalize();
            data->~_Tp();
            blk->~GCBlock();

            if (! blk && m_head) {
                blk = m_head;
                m_head = 0;
            }
        }
    }

    inline int newAllocatedBlocks() const { return m_new_allocated_blocks; }
    inline int freeBlocks() const { return m_free_blocks; }

    inline _Tp *operator()()
    {
        GCBlock *previous = m_current;
        void *where = 0;

        if (! m_free) {
            Q_ASSERT (m_free_blocks == 0);
            where = pool.allocate(sizeof(GCBlock) + sizeof(_Tp));
            ++m_new_allocated_blocks;
            (void) new (reinterpret_cast<char*>(where) + sizeof(GCBlock)) _Tp();
        } else {
            --m_free_blocks;
            where = m_free;
            m_free = m_free->next;

            if (! m_free)
                m_force_gc = true;
        }

        m_current = new (where) GCBlock(0);

        if (! previous) {
            Q_ASSERT(! m_head);
            m_head = m_current;
        } else {
            previous->next = m_current;
            m_current->generation = m_head->generation;
        }

        return reinterpret_cast<_Tp*> (m_current->data());
    }

    inline void release(_Tp *ptr)
    {
        GCBlock::get(ptr)->released = true;
    }

    inline bool blocked() const
    {
        return m_blocked_gc;
    }

    inline bool blockGC(bool block)
    {
        bool was = m_blocked_gc;
        m_blocked_gc = block;
        return was;
    }

    inline bool poll()
    {
        if (m_blocked_gc || ! m_head)
            return false;

        else if (m_free && ! m_free->next)
            return true;

        else if (m_force_gc) {
            m_force_gc = false;
            return true;
        }

        return m_new_allocated_blocks >= MaxNumberOfBlocks;
    }

    inline int generation(_Tp *ptr) const
    { return GCBlock::get(ptr)->generation; }

    inline GCBlock *head() const
    { return m_head; }

    void sweep(int generation)
    {
        GCBlock *blk = m_head;
        m_current = 0;

        m_new_allocated_blocks = 0;

        while (blk != 0) {
            if (blk->generation != generation) {
                if (m_current)
                    m_current->next = blk->next;

                GCBlock *tmp = blk;
                blk = blk->next;    // advance the pointer

                tmp->next = m_free; // prepend the node to the free list...
                m_free = tmp;
                ++m_free_blocks;

                if (m_free == m_head)
                    m_head = 0;

                _Tp *data = reinterpret_cast<_Tp *>(tmp->data());
                data->finalize(); // we need it
                tmp->~GCBlock();
            } else {
                m_current = blk;
                blk = blk->next;
            }
        }

        if (! m_current)
            m_head = m_current;
    }

private:
    Q_DISABLE_COPY(GCAlloc)
};

} // namespace QScript

#endif // QSCRIPT_GC_H
