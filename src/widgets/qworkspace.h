/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.h#11 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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
#include "qwidgetlist.h"


#if defined(Q_TEMPLATEDLL)
/*
  Gives moc syntax error
template class Q_EXPORT QList<QWorkspaceChild>;
*/
#endif


class Q_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
public:
    QWorkspace( QWidget *parent=0, const char *name=0 );
    ~QWorkspace();

    void activateClient( QWidget* w);
    void maximizeClient( QWidget* w);
    void minimizeClient( QWidget* w);
    void normalizeClient( QWidget* w);
    QWidget* activeClient() const;

signals:
    void clientActivated( QWidget* w);

protected:
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );

private:
    QWorkspaceChild* active;
    QList<QWorkspaceChild> windows;
    void insertIcon( QWidget* w);
    void removeIcon( QWidget* w);
    QList<QWidget> icons;
    void place( QWorkspaceChild* );

    QWorkspaceChild* findChild( QWidget* w);
    void showMaxHandles();
    void hideMaxHandles();
    QWorkspaceChild* maxClient;
    QRect maxRestore;

    int px;
    int py;

    void layoutIcons();

};
#endif
