/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.h#20 $
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

#ifndef QT_H
#include <qwidget.h>
#include <qwidgetlist.h>
#endif // QT_H

class QWorkspaceChild;
class QWorkspaceData;
class QShowEvent;

class Q_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
public:
    QWorkspace( QWidget *parent=0, const char *name=0 );
    ~QWorkspace();

    QWidget* activeClient() const;
    QWidgetList clientList() const;

signals:
    void clientActivated( QWidget* w);

    
public slots:
    void cascade();
    void tile();
    
protected:
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );
    void showEvent( QShowEvent *e );

private slots:
    void closeActiveClient();
    void normalizeActiveClient();
    void minimizeActiveClient();
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );
    void operationMenuActivated( int );
    void operationMenuAboutToShow();
    void activateNextClient();
    void activatePreviousClient();

private:
    void insertIcon( QWidget* w);
    void removeIcon( QWidget* w);
    void place( QWidget* );

    QWorkspaceChild* findChild( QWidget* w);
    void showMaximizeControls();
    void hideMaximizeControls();
    void layoutIcons();
    QWorkspaceData* d;
    void activateClient( QWidget* w, bool change_focus = TRUE );
    void showClient( QWidget* w);
    void maximizeClient( QWidget* w);
    void minimizeClient( QWidget* w);
    void normalizeClient( QWidget* w);
    QPopupMenu* popup;
    friend QWorkspaceChild;


};
#endif
