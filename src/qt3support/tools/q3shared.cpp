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

#include "q3shared.h"

QT_BEGIN_NAMESPACE

/*!
  \class Q3Shared
  \brief The Q3Shared class is used internally for implementing shared classes.
  \compat

  Use QSharedData and QSharedDataPointer instead.
*/

/*!
  \fn Q3Shared::Q3Shared()

  Constructs a Q3Shared object with a reference count of 1.
*/

/*!
  \fn void Q3Shared::ref()

  Increments the reference count.
*/

/*!
  \fn bool Q3Shared::deref()
  Decrements the reference count and returns true if
  any references remain. 
*/

QT_END_NAMESPACE
