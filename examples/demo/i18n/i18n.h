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

#ifndef I18N_H
#define I18N_H

#include <qmainwindow.h>

class QWorkspace;
class Q3Action;
class QPopupMenu;
class Wrapper;


class I18nDemo : public QMainWindow
{
    Q_OBJECT

public:
    I18nDemo(QWidget *, const char * = 0);
    ~I18nDemo();

    void initActions();
    void initMenuBar();

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

    QWorkspace *workspace;
    Q3Action *actionClose, *actionCloseAll, *actionTile, *actionCascade;
    QPopupMenu *windowMenu, *newMenu;
    Wrapper *lastwrapper;


public slots:
    void newSlot(int);
    void windowSlot(int);
    void windowActivated(QWidget *);
    void closeSlot();
    void closeAllSlot();
    void tileSlot();
    void cascadeSlot();
    void wrapperDead();
};


#endif // I18N_H
