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

#include "qdesigner_propertycommentcommand_p.h"
#include "metadatabase_p.h"
#include "qdesigner_propertyeditor_p.h"

#include <QtDesigner/QtDesigner>
#include <QtGui/QApplication>
#include <qdebug.h>

namespace qdesigner_internal {

SetPropertyCommentCommand::Entry::Entry(QObject* object, const QString &oldCommentValue) :
    m_object(object),
    m_oldCommentValue(oldCommentValue)
{
}

SetPropertyCommentCommand::SetPropertyCommentCommand(QDesignerFormWindowInterface *formWindow) :
     QDesignerFormWindowCommand(QString(), formWindow),
     m_propertyType(QVariant::Invalid)
{
}

bool SetPropertyCommentCommand::init(QObject *object, const QString &propertyName, const QString &newCommentValue)
{
    m_propertyName = propertyName;
    m_newCommentValue = newCommentValue;

    m_Entries.clear();
    if (!add(object))
        return false;

    setDescription();
    return true;
}

void SetPropertyCommentCommand::setDescription()
{
    if (m_Entries.size() == 1) {
        setText(QApplication::translate("Command", "changed comment of '%1' of '%2'").arg(m_propertyName).arg(m_Entries[0].m_object->objectName()));
    } else {
        setText(QApplication::translate("Command", "changed comment of '%1' of %2 objects").arg(m_propertyName).arg(m_Entries.size()));
    }
}

bool SetPropertyCommentCommand::init(const ObjectList &list, const QString &propertyName, const QString &newCommentValue)
{
    m_propertyName = propertyName;
    m_newCommentValue = newCommentValue;

    m_Entries.clear();
    foreach (QObject *o, list) {
        add(o);
    }

    if (m_Entries.empty())
        return false;

    setDescription();
    return true;
}


bool SetPropertyCommentCommand::add(QObject *object)
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    if (!sheet)
        return false;

    const int index = sheet->indexOf(m_propertyName);
    if (index == -1 || !sheet->isVisible(index))
        return false;

    // Set or check type
    const QVariant::Type propertyType = sheet->property(index).type();
    if (m_Entries.empty()) {
        m_propertyType = propertyType;
    } else {
        if ( propertyType != m_propertyType)
            return false;
    }

    const QString oldCommentValue = propertyComment(core, object, m_propertyName);

    m_Entries.push_back(Entry(object, oldCommentValue));
    return true;
}


int SetPropertyCommentCommand::id() const
{
    return 1968;
}

bool SetPropertyCommentCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    // check property name and list of objects
    const SetPropertyCommentCommand *cmd = static_cast<const SetPropertyCommentCommand*>(other);

    if (cmd->m_propertyName != m_propertyName)
         return false;

    const int numEntries = m_Entries.size();
    if (numEntries != cmd->m_Entries.size()) {
        return false;
    }

    for (int i = 0; i < numEntries; i++) {
        if (m_Entries[i].m_object != cmd->m_Entries[i].m_object)
            return false;
    }

    m_newCommentValue = cmd->m_newCommentValue;
    return true;

}

void SetPropertyCommentCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QDesignerPropertyEditor *designerPropertyEditor = qobject_cast<QDesignerPropertyEditor *>(core->propertyEditor());
    Q_ASSERT(designerPropertyEditor);
    QObject* propertyEditorObject = designerPropertyEditor->object();
    // Set m_newCommentValue and update property editor
    const EntryList::const_iterator cend = m_Entries.end();
    for (EntryList::const_iterator it = m_Entries.begin(); it != cend; ++it) {
        if (QObject *object = it->m_object) { // might have been deleted
            setPropertyComment(core, object, m_propertyName, m_newCommentValue);
            if (object == propertyEditorObject)
                designerPropertyEditor->setPropertyComment(m_propertyName, m_newCommentValue);
        }
    }
}

void SetPropertyCommentCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QDesignerPropertyEditor *designerPropertyEditor = qobject_cast<QDesignerPropertyEditor *>(core->propertyEditor());
    Q_ASSERT(designerPropertyEditor);
    QObject* propertyEditorObject = designerPropertyEditor->object();

    // Set stored old value and update property editor
    const EntryList::const_iterator cend = m_Entries.end();
    for (EntryList::const_iterator it = m_Entries.begin(); it != cend; ++it) {
        if (QObject *object = it->m_object) {
            setPropertyComment(core, object, m_propertyName, it->m_oldCommentValue);
            if (object == propertyEditorObject)
                designerPropertyEditor->setPropertyComment(m_propertyName, it->m_oldCommentValue);
        }
    }
}

} // namespace qdesigner_internal
