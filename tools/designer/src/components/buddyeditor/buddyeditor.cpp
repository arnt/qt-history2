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

#include "buddyeditor.h"

#include <abstractformwindow.h>

#include <qtundo.h>
#include <qdesigner_command.h>

#include <QtGui/QLabel>

/*******************************************************************************
** BuddyConnection
*/

class BuddyConnection : public Connection
{
public:
    BuddyConnection(BuddyEditor *edit, QWidget *source, QWidget *target)
        : Connection(edit, source, target) {}
    virtual void inserted();
    virtual void removed();

    BuddyEditor *buddyEditor() const { return qobject_cast<BuddyEditor*>(edit()); }
};

void BuddyConnection::inserted()
{
    QWidget *source = widget(EndPoint::Source);
    QWidget *target = widget(EndPoint::Target);
    if (qobject_cast<QLabel*>(source) == 0) {
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
    if (qobject_cast<QLabel*>(source) == 0) {
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

    if (state() == Editing) {
        QLabel *label = qobject_cast<QLabel*>(w);
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
