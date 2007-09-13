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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_SCRIPTCOMMAND_H
#define QDESIGNER_SCRIPTCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT ScriptCommand: public QDesignerFormWindowCommand
{
    ScriptCommand(const ScriptCommand &);
    ScriptCommand& operator=(const ScriptCommand &);

public:
    explicit ScriptCommand(QDesignerFormWindowInterface *formWindow);

    typedef QList<QObject *> ObjectList;
    bool init(const ObjectList &list, const QString &script);

    virtual void redo();
    virtual void undo();

private:
    typedef QPair<QPointer<QObject>, QString> ObjectScriptPair;
    typedef QList<ObjectScriptPair> ObjectScriptList;
    ObjectScriptList m_oldValues;
    QString m_script;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_SCRIPTCOMMAND_H
