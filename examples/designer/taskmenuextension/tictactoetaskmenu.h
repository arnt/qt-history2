#ifndef TICTACTOETASKMENU_H
#define TICTACTOETASKMENU_H

#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QExtensionFactory>

class QAction;
class QExtensionManager;
class TicTacToe;

class TicTacToeTaskMenu: public QObject, public QDesignerTaskMenuExtension
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


class TicTacToeTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    TicTacToeTaskMenuFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
