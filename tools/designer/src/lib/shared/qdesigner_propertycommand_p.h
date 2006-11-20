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

#ifndef QDESIGNER_PROPERTYCOMMAND_H
#define QDESIGNER_PROPERTYCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QVariant>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT SetPropertyCommand: public QDesignerFormWindowCommand
{

public:
    SetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    void init(QObject *object, const QString &propertyName, const QVariant &newValue);

    QObject *object() const;

    QWidget *widget() const;
    QWidget *parentWidget() const;

    inline QVariant oldValue() const
    { return m_oldValue; }

    inline void setOldValue(const QVariant &oldValue)
    { m_oldValue = oldValue; }

    inline QVariant newValue() const
    { return m_newValue; }

    inline void setNewValue(const QVariant &newValue)
    { m_newValue = newValue; }
    
    int id() const;
    bool mergeWith(const QUndoCommand *other);

public:
    virtual void redo();
    virtual void undo();

private:
    QString m_propertyName;
    int m_index;
    QPointer<QObject> m_object;
    QPointer<QWidget> m_parentWidget;
    QDesignerPropertySheetExtension *m_propertySheet;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_changed;
};

class QDESIGNER_SHARED_EXPORT ResetPropertyCommand: public QDesignerFormWindowCommand
{

public:
    ResetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    void init(QObject *object, const QString &propertyName);

    QObject *object() const;
    QObject *parentObject() const;

    virtual void redo();
    virtual void undo();

protected:
    virtual bool mergeWith(const QUndoCommand *other) { Q_UNUSED(other); return false; }

private:
    QString m_propertyName;
    int m_index;
    QPointer<QObject> m_object;
    QPointer<QObject> m_parentObject;
    QDesignerPropertySheetExtension *m_propertySheet;
    QVariant m_oldValue;
    bool m_changed;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_PROPERTYCOMMAND_H
