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

#ifndef CONNECTIONVIEWERIMPL_H
#define CONNECTIONVIEWERIMPL_H

#include "connectionviewer.h"
#include "metadatabase.h"

#include <qmap.h>

class FormWindow;

class ConnectionViewer : public ConnectionViewerBase
{
    Q_OBJECT

public:
    ConnectionViewer( QWidget *parent, FormWindow *fw );

protected slots:
    void editConnection();
    void disconnectConnection();
    void currentConnectionChanged( QListViewItem *i );

private:
    void readConnections();
    FormWindow *formWindow;
    QMap<QListViewItem*, MetaDataBase::Connection> connections;
    
};

#endif
