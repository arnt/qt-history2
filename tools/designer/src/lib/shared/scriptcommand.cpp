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

#include "scriptcommand_p.h"
#include "metadatabase_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

ScriptCommand::ScriptCommand(QDesignerFormWindowInterface *formWindow) :
    QDesignerFormWindowCommand(QObject::tr("Change script"), formWindow)
{
}

bool ScriptCommand::init(const ObjectList &list, const QString &script)
{
    MetaDataBase *metaDataBase = qobject_cast<MetaDataBase*>(formWindow()->core()->metaDataBase());
    if (!metaDataBase)
        return false;

    // Save old values
    m_oldValues.clear();
    foreach (QObject *obj, list) {
        const MetaDataBaseItem* item = metaDataBase->metaDataBaseItem(obj);
        if (!item)
            return false;
        m_oldValues.push_back(ObjectScriptPair(obj, item->script()));
    }
    m_script = script;
    return true;
}

void ScriptCommand::redo()
{
    MetaDataBase *metaDataBase = qobject_cast<MetaDataBase*>(formWindow()->core()->metaDataBase());
    Q_ASSERT(metaDataBase);

    ObjectScriptList::const_iterator cend = m_oldValues.constEnd();
    for (ObjectScriptList::const_iterator it = m_oldValues.constBegin();it != cend; ++it )  {
        if (it->first)
            metaDataBase->metaDataBaseItem(it->first)->setScript(m_script);
    }
}

void ScriptCommand::undo()
{
    MetaDataBase *metaDataBase = qobject_cast<MetaDataBase*>(formWindow()->core()->metaDataBase());
    Q_ASSERT(metaDataBase);

    ObjectScriptList::const_iterator cend = m_oldValues.constEnd();
    for (ObjectScriptList::const_iterator it = m_oldValues.constBegin();it != cend; ++it )  {
        if (it->first)
            metaDataBase->metaDataBaseItem(it->first)->setScript(it->second);
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
