/****************************************************************************
**
** Definition of QToolBarButton widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTOOLBARBUTTON_P_H
#define QTOOLBARBUTTON_P_H

#include <qabstractbutton.h>

class QMenu;
class QToolBarButtonPrivate;

class QToolBarButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QToolBarButton);

public:
    QToolBarButton(QWidget *parent);
    ~QToolBarButton();

    void setUsesTextLabel(bool enable);
    bool usesTextLabel() const;

    void setMenu(QMenu *menu);
    QMenu *menu() const;
    void showMenu();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    bool hitButton(const QPoint &pos) const;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void actionEvent(QActionEvent *event);

    void nextCheckState(){} // we do not want the button to change state on its own, only through the action
};

#endif // QTOOLBARBUTTON_P_H
