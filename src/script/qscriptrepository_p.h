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

#ifndef QSCRIPTREPOSITORY_P_H
#define QSCRIPTREPOSITORY_P_H

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

#include "qscriptbuffer_p.h"

namespace QScript {

template <typename Tp>
class Repository
{
public:
    inline Repository() { cache.reserve(32); }
    inline ~Repository() { qDeleteAll(cache); }

    inline Tp *get()
    {
        if (cache.isEmpty())
            return new Tp();

        return cache.takeLast();
    }

    inline void release(Tp *item)
    { cache.append(item); }

private:
    Buffer<Tp*> cache;

private:
    Q_DISABLE_COPY(Repository)
};

} // namespace QScript

#endif

