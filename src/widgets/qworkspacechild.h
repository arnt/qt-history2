/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspacechild.h#2 $
**
** Definition of the QChildWindow class
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

#ifndef QWORKSPACECHILD_H
#define QWORKSPACECHILD_H

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
class QToolButton;
class QLabel;
class QWorkspace;

class QWorkspaceChild : public QFrame
{
    Q_OBJECT
public:
    QWorkspaceChild( QWidget* client, QWorkspace *parent=0, const char *name=0 );
    ~QWorkspaceChild();
    
    void setActive( bool );
    bool isActive() const;
    
    QWidget* clientWidget() const;

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );

    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );
    
private:
    QWidget* clientw;
    bool buttonDown;
    int mode;
    QPoint moveOffset;
    QToolButton* closeB;
    QToolButton* maxB;
    QToolButton* iconB;
    QLabel* titleL;
    bool act;

};
#endif
