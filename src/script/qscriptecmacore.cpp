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

#include "qscriptengine.h"
#include "qscriptvalue_p.h"
#include "qscriptobject_p.h"
#include "qscriptengine_p.h"
#include "qscriptecmacore_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"

namespace QScript { namespace Ecma {

Core::Core(QScriptEnginePrivate *engine)
    : m_engine(engine)
{
    this->length = 1;
    ctor.invalidate();
}

Core::~Core()
{
}


} // namespace Ecma


} // namespace QScript
