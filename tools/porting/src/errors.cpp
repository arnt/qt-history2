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

#include "errors.h"

QT_STATIC_CONST_IMPL Error& Errors::InternalError = Error( 1, -1, QLatin1String("Internal Error") );
QT_STATIC_CONST_IMPL Error& Errors::SyntaxError = Error( 2, -1, QLatin1String("Syntax Error before '%1'") );
QT_STATIC_CONST_IMPL Error& Errors::ParseError = Error( 3, -1, QLatin1String("Parse Error before '%1'") );
