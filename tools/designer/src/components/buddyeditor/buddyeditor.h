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

#ifndef BUDDYEDITOR_H
#define BUDDYEDITOR_H

#include <connectionedit.h>
#include <abstractformwindow.h>
#include "buddyeditor_global.h"

class BuddyConnection : public Connection
{
    Q_OBJECT

public:
    BuddyConnection(ConnectionEdit *edit);
    ~BuddyConnection();
};

class QT_BUDDYEDITOR_EXPORT BuddyEditor : public ConnectionEdit
{
    Q_OBJECT

public:    
    BuddyEditor(AbstractFormWindow *form_window, QWidget *parent);

    BuddyConnection *findConnection(QWidget *source, QWidget *destination);
    void addConnection(QWidget *source, QWidget *destination, const Connection::HintList &hints);
    void deleteConnection(QWidget *source, QWidget *destination);
        
protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;

private:    
    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    
    AbstractFormWindow *m_form_window;
};

#endif
