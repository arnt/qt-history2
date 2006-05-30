/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TICTACTOETASKMENU_H
#define TICTACTOETASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;
class TicTacToe;

class TicTacToeTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    TicTacToeTaskMenu(TicTacToe *tic, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editState();

private:
    QAction *editStateAction;
    TicTacToe *ticTacToe;
};

class TicTacToeTaskMenuFactory : public QExtensionFactory
{
    Q_OBJECT

public:
    TicTacToeTaskMenuFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
