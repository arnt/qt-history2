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

#ifndef QSCRIPTECMACORE_P_H
#define QSCRIPTECMACORE_P_H

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

#include "qscriptfunction_p.h"

namespace QScript { namespace Ecma {

class Core: public QScriptFunction
{
public:
    Core(QScriptEnginePrivate *engine);
    virtual ~Core();

    inline QScriptEnginePrivate *engine() const
    { return m_engine; }

public: // attributes
    QScriptValueImpl ctor;
    QScriptValueImpl publicPrototype;

private:
    QScriptEnginePrivate *m_engine;
};

} } // namespace QScript::Ecma

#endif

