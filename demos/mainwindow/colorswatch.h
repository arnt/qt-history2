#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <qdockwindow.h>

class QAction;
class QActionGroup;
class QMenu;

class ColorSwatch : public QDockWindow
{
    Q_OBJECT

    QAction *showHideAction;

    QAction *closableAction;
    QAction *movableAction;
    QAction *floatableAction;
    QAction *topLevelAction;

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
    ColorSwatch(const QString & colorName, QMainWindow *parent, Qt::WFlags flags);

    QMenu *menu;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void polishEvent(QEvent *);

    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void allow(Qt::DockWindowArea area, bool allow);
    void place(Qt::DockWindowArea area, bool place);

private slots:
    void showHide();

    void changeClosable(bool on);
    void changeMovable(bool on);
    void changeFloatable(bool on);

    void changeTopLevel(bool topLevel);

    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);
};

#endif // COLORSWATCH_H
