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
{}

BuddyConnection::~BuddyConnection()
{}

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
    Connection *con = new BuddyConnection(this);
    con->setSource(source);
    con->setDestination(destination);
    
    CEItem *item = widgetItem(source);
    if (item)
        item->disableSelect(true);
        
    return con;
}

BuddyConnection *BuddyEditor::findConnection(QWidget *source, QWidget *destination)
{
    for (int i = 0; i < connectionCount(); ++i) {
        Connection *con = connection(i);
        if (con->source() == source && con->destination() == destination)
            return qt_cast<BuddyConnection*>(con);
    }
    return 0;
}

void BuddyEditor::addConnection(QWidget *source, QWidget *destination, 
                                const Connection::HintList &hints)
{
    blockSignals(true);
    Connection *con = createConnection(source, destination);
    initConnection(con, hints);
    blockSignals(false);
}

void BuddyEditor::deleteConnection(QWidget *source, QWidget *destination)
{
    Connection *con = findConnection(source, destination);
    blockSignals(true);
    if (con != 0)
        ConnectionEdit::deleteConnection(con);
    blockSignals(false);
}
