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

#ifndef QSCRIPTCLASSINFO_P_H
#define QSCRIPTCLASSINFO_P_H

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

class QScriptEngine;
#include "qscriptglobals_p.h"
#include "qscriptclassdata_p.h"
#include <QtCore/qshareddata.h>

class QScriptClassInfo
{
public:
    inline QScriptClassInfo() { }
    inline ~QScriptClassInfo() { }

    inline QScript::Type type() const
        { return m_type; }
    inline QScriptEngine *engine() const
        { return m_engine; }
    inline QString name() const
        { return m_name; }

    inline void setData(QExplicitlySharedDataPointer<QScriptClassData> data)
        { m_data = data; }
    QExplicitlySharedDataPointer<QScriptClassData> data() const
        { return m_data; }

private:
    QScript::Type m_type;
    QScriptEngine *m_engine;
    QString m_name;
    QExplicitlySharedDataPointer<QScriptClassData> m_data;

private:
    friend class QScriptEnginePrivate;
    // Q_DISABLE_COPY(QScriptClassInfo) // ### disable copy
};

#endif // QSCRIPTCLASSINFO_P_H
