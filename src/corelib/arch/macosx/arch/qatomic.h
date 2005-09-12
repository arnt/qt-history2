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

#ifndef MACOSX_QATOMIC_H
#define MACOSX_QATOMIC_H

#include <QtCore/qglobal.h>

#if defined(_ARCH_PPC)
#  include "../../powerpc/arch/qatomic.h"
#endif //_ARCH_PPC

#if defined(__i386__)
#  include "../../i386/arch/qatomic.h"
#endif //__i386__

#endif // MACOSX_QATOMIC_H
