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
#include "errors.h"
#include <klocale.h>

QT_STATIC_CONST_IMPL Error& Errors::InternalError = Error( 1, -1, i18n("Internal Error") );
QT_STATIC_CONST_IMPL Error& Errors::SyntaxError = Error( 2, -1, i18n("Syntax Error before '%1'") );
QT_STATIC_CONST_IMPL Error& Errors::ParseError = Error( 3, -1, i18n("Parse Error before '%1'") );
