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

/*
  tr.h
*/

#ifndef TR_H
#define TR_H

#include <qstring.h>

QT_BEGIN_NAMESPACE

inline QString tr( const char *sourceText, const char * /* comment */ = 0 )
{
    return QString( sourceText );
}

QT_END_NAMESPACE

#endif
