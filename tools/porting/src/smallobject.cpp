/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
** Copyright (C) 2001-2004 Roberto Raggi
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "smallobject.h"

int block_t::N = 0;
static pool mem_pool;

void *SmallObject::operator new(size_t size)
{
    return mem_pool.allocate(size);
}

void SmallObject::operator delete(void * /*p*/ )
{
}
