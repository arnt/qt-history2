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

#include <QtCore/qdebug.h>

#include "buddyeditor.h"

#include <abstractformwindow.h>
#include <qextensionmanager.h>
#include <propertysheet.h>
#include <abstractformeditor.h>

#include <qtundo.h>
#include <qdesigner_command.h>
#include <qdesigner_widget.h>

/*******************************************************************************
** BuddyConnection
*/

class BuddyConnection : public Connection
{
public:
    BuddyConnection(BuddyEditor *edit, QWidget *source, QWidget *target)
        : Connection(edit, source, target) {}
    BuddyConnection(BuddyEditor *edit)
        : Connection(edit) {}
    virtual void inserted();
    virtual void removed();

    BuddyEditor *buddyEditor() const { return qobject_cast<BuddyEditor*>(edit()); }
};

void BuddyConnection::inserted()
{
    QWidget *source = widget(EndPoint::Source);
    QWidget *target = widget(EndPoint::Target);
    if (qobject_cast<QDesignerLabel*>(source) == 0) {
        qWarning("BuddyConnection::inserted(): not a label");
        return;
    }
    SetPropertyCommand command(buddyEditor()->formWindow());
    command.init(source, QLatin1String("buddy"), target->objectName());
    command.redo();
}

void BuddyConnection::removed()
{
    QWidget *source = widget(EndPoint::Source);
    if (qobject_cast<QDesignerLabel*>(source) == 0) {
        qWarning("BuddyConnection::removed(): not a label");
        return;
    }
    SetPropertyCommand command(buddyEditor()->formWindow());
    command.init(source, QLatin1String("buddy"), QString());
    command.redo();
}

/*******************************************************************************
** BuddyEditor
*/

BuddyEditor::BuddyEditor(AbstractFormWindow *form, QWidget *parent)
    : ConnectionEdit(parent, form)
{
    m_formWindow = form;
}

QWidget *BuddyEditor::widgetAt(const QPoint &pos) const
{
    QWidget *w = ConnectionEdit::widgetAt(pos);

    if (w == m_formWindow->mainContainer())
        return 0;

    if (state() == Editing) {
        QDesignerLabel *label = qobject_cast<QDesignerLabel*>(w);
        if (label == 0)
            return 0;
        int cnt = connectionCount();
        for (int i = 0; i < cnt; ++i) {
            Connection *con = connection(i);
            if (con->widget(EndPoint::Source) == w)
                return 0;
        }
    }

    return w;
}

Connection *BuddyEditor::createConnection(QWidget *source, QWidget *destination)
{
    Connection *con = new BuddyConnection(this, source, destination);
    return con;
}

AbstractFormWindow *BuddyEditor::formWindow() const
{
    return m_formWindow;
}

static QString buddy(QDesignerLabel *label, AbstractFormEditor *core)
{
    IPropertySheet *sheet = qt_extension<IPropertySheet*>(core->extensionManager(), label);
    if (sheet == 0)
        return QString();
    int prop_idx = sheet->indexOf(QLatin1String("buddy"));
    if (prop_idx == -1)
        return QString();
    return sheet->property(prop_idx).toString();
}

void BuddyEditor::setBackground(QWidget *background)
{
    ConnectionEdit::setBackground(background);
    clear();

    QList<QDesignerLabel*> label_list = qFindChildren<QDesignerLabel*>(background);
    foreach (QDesignerLabel *label, label_list) {
        QString buddy_name = buddy(label, m_formWindow->core());
        if (buddy_name.isEmpty())
            continue;
        QWidget *target = qFindChild<QWidget*>(background, buddy_name);
        if (target == 0)
            continue;
        
        BuddyConnection *con = new BuddyConnection(this);
        con->setEndPoint(EndPoint::Source, label, widgetRect(label).center());
        con->setEndPoint(EndPoint::Target, target, widgetRect(target).center());
        addConnection(con);
    }
}

