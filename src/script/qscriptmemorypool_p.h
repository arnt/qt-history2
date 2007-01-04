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

#ifndef QSCRIPTMEMORYPOOL_P_H
#define QSCRIPTMEMORYPOOL_P_H

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
#include <string.h>

namespace QScript {

class MemoryPool
{
public:
    enum { maxBlockCount = -1 };
    enum { defaultBlockSize = 1 << 16 };

    MemoryPool() {
        m_blockIndex = maxBlockCount;
        m_currentIndex = 0;
        m_storage = 0;
        m_currentBlock = 0;
    }

    ~MemoryPool() {
        for (int index = 0; index < m_blockIndex + 1; ++index)
            qFree(m_storage[index]);

        qFree(m_storage);
    }

    char *allocate(int bytes) {
        if (m_currentBlock == 0 || defaultBlockSize < m_currentIndex + bytes) {
            ++m_blockIndex;

            m_storage = reinterpret_cast<char**>(qRealloc(m_storage, sizeof(char*) * (1 + m_blockIndex)));
            m_currentBlock = m_storage[m_blockIndex] = reinterpret_cast<char*>(qMalloc(defaultBlockSize));
            ::memset(m_currentBlock, 0, defaultBlockSize);

            m_currentIndex = 0;
        }

        char *p = reinterpret_cast<char *>
            (m_currentBlock + m_currentIndex);

        m_currentIndex += bytes;

        return p;
    }

private:
    int m_blockIndex;
    int m_currentIndex;
    char *m_currentBlock;
    char **m_storage;

private:
    Q_DISABLE_COPY(MemoryPool)
};

} // namespace QScript

#endif

