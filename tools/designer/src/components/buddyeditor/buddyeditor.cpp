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
#include "buddyeditor.h"

/*******************************************************************************
** BuddyConnection
*/

BuddyConnection::BuddyConnection(ConnectionEdit *edit)
    : Connection(edit)
{
}

/*******************************************************************************
** BuddyEditor
*/

BuddyEditor::BuddyEditor(AbstractFormWindow *form_window, QWidget *parent)
    : ConnectionEdit(parent)
{
    m_form_window = form_window;
}

QWidget *BuddyEditor::widgetAt(const QPoint &pos) const
{
    QWidget *w = ConnectionEdit::widgetAt(pos);
    
    if (mode() == EditMode) {
        if (qt_cast<QLabel*>(w) == 0)
            return 0;
    }
        
    return w;
}

Connection *BuddyEditor::createConnection(QWidget *source, QWidget *destination)
{
    Connection *con = ConnectionEdit::createConnection(source, destination);
    widgetItem(con->source())->disableSelect(true);
    widgetItem(con->destination())->disableSelect(true);
    return con;
}

