/****************************************************************************
** $Id: //depot/qt/main/tests/mdi/qworkspace.h#3 $
**
** Definition of the QWorkspace class
**
** Created : 990210
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWORKSPACE_H
#define QWORKSPACE_H

//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

#include <qframe.h>
#include <qlist.h>
#include "qworkspacechild.h"


class QWorkspaceData;

class Q_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
public:
    QWorkspace( QWidget *parent=0, const char *name=0 );
    ~QWorkspace();

    void activateClient( QWidget* w);
    void showClient( QWidget* w);
    void maximizeClient( QWidget* w);
    void minimizeClient( QWidget* w);
    void normalizeClient( QWidget* w);
    QWidget* activeClient() const;

signals:
    void clientActivated( QWidget* w);

protected:
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private slots:
    void closeActive();
    void normalizeActive();
    
private:
    void insertIcon( QWidget* w);
    void removeIcon( QWidget* w);
    void place( QWidget* );

    QWorkspaceChild* findChild( QWidget* w);
    void showMaxHandles();
    void hideMaxHandles();
    void layoutIcons();
    QWorkspaceData* d;


};
#endif
