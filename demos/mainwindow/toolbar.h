#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <qtoolbar.h>

class QAction;
class QActionGroup;
class QMenu;
class QSpinBox;

class ToolBar : public QToolBar
{
    Q_OBJECT

    QSpinBox *spinbox;
    QAction *spinboxAction;

    QAction *showHideAction;
    QAction *orderAction;
    QAction *randomizeAction;
    QAction *addSpinBoxAction;
    QAction *removeSpinBoxAction;

    QAction *movableAction;

    QActionGroup *allowedAreasActions;
    QAction *allowLeftAction;
    QAction *allowRightAction;
    QAction *allowTopAction;
    QAction *allowBottomAction;

    QActionGroup *areaActions;
    QAction *leftAction;
    QAction *rightAction;
    QAction *topAction;
    QAction *bottomAction;

public:
    ToolBar(QMainWindow *parent);

    QMenu *menu;

protected:
    virtual void polishEvent(QEvent *);

    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void allow(Qt::ToolBarArea area, bool allow);
    void place(Qt::ToolBarArea area, bool place);

private slots:
    void showHide();
    void order();
    void randomize();
    void addSpinBox();
    void removeSpinBox();

    void changeMovable(bool movable);

    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);

};

#endif // TOOLBAR_H
