/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef CONNECTIONEDITORIMPL_H
#define  CONNECTIONEDITORIMPL_H

#include "connectioneditor.h"
#include "metadatabase.h"

#include <qmap.h>

class QListViewItem;
class FormWindow;

class ConnectionEditor : public ConnectionEditorBase
{
    Q_OBJECT

public:
    ConnectionEditor( QWidget *parent, QObject* sender, QObject* receiver, FormWindow *fw );
    ~ConnectionEditor();


protected:
    void signalChanged();
    void connectClicked();
    void disconnectClicked();
    void okClicked();
    void cancelClicked();
    void slotsChanged();
    void connectionsChanged();
    void addSlotClicked();
    
private:
    bool ignoreSlot( const char* ) const;
    struct Connection
    {
	QCString signal, slot;
    };

    QMap<QListViewItem*, Connection> connections;
    QValueList<MetaDataBase::Connection> oldConnections;
    QObject* sender;
    QObject* receiver;
    FormWindow *formWindow;

};

#endif
