//depot/qt/main/tools/designer/src/components/buddyeditor/buddyeditor.cpp#3 - edit change 165263 (text)
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

#include <QLabel>
#include <qtundo.h>
#include "buddyeditor.h"

#include <formwindow.h>
#include <command.h>

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
    
    BuddyEditor *buddyEditor() const { return qt_cast<BuddyEditor*>(edit()); }
};

void BuddyConnection::inserted()
{
    QWidget *source = widget(EndPoint::Source);
    QWidget *target = widget(EndPoint::Target);
    if (qt_cast<QLabel*>(source) == 0) {
        qWarning("BuddyConnection::inserted(): not a label");
        return;
    }
    SetPropertyCommand command(buddyEditor()->form());
    command.init(source, QLatin1String("buddy"), target->objectName());
    command.redo();
}

void BuddyConnection::removed()
{
    QWidget *source = widget(EndPoint::Source);
    if (qt_cast<QLabel*>(source) == 0) {
        qWarning("BuddyConnection::removed(): not a label");
        return;
    }
    SetPropertyCommand command(buddyEditor()->form());
    command.init(source, QLatin1String("buddy"), QString());
    command.redo();
}

/*******************************************************************************
** BuddyEditor
*/

BuddyEditor::BuddyEditor(FormWindow *form, QWidget *parent)
    : ConnectionEdit(parent, form)
{
    m_form = form;
}

QWidget *BuddyEditor::widgetAt(const QPoint &pos) const
{
    QWidget *w = ConnectionEdit::widgetAt(pos);
    
    if (state() == Editing && qt_cast<QLabel*>(w) == 0)
        return 0;
        
    return w;
}

Connection *BuddyEditor::createConnection(QWidget *source, QWidget *destination)
{
    Connection *con = new BuddyConnection(this, source, destination);
    return con;
}
