#include "colorswatch.h"

#include <qaction.h>
#include <qevent.h>
#include <qframe.h>
#include <qmenu.h>

ColorSwatch::ColorSwatch(const QString &colorName, QMainWindow *parent, Qt::WFlags flags)
    : QDockWindow(parent, flags)
{
    setObjectName(colorName + QLatin1String(" Dock Window"));
    setWindowTitle(objectName());

    QFrame *swatch = new QFrame(this);
    swatch->setFrameStyle(QFrame::Box | QFrame::Sunken);
    swatch->setMinimumSize(150, 100);

    QPalette pal = swatch->palette();
    pal.setColor(swatch->backgroundRole(), QColor(colorName));
    swatch->setPalette(pal);

    setWidget(swatch);

    showHideAction = new QAction(this);
    connect(showHideAction, SIGNAL(triggered()), SLOT(showHide()));

    closableAction = new QAction(tr("Closable"), this);
    closableAction->setCheckable(true);
    connect(closableAction, SIGNAL(checked(bool)), SLOT(changeClosable(bool)));

    movableAction = new QAction(tr("Movable"), this);
    movableAction->setCheckable(true);
    connect(movableAction, SIGNAL(checked(bool)), SLOT(changeMovable(bool)));

    floatableAction = new QAction(tr("Floatable"), this);
    floatableAction->setCheckable(true);
    connect(floatableAction, SIGNAL(checked(bool)), SLOT(changeFloatable(bool)));

    topLevelAction = new QAction(tr("Top Level"), this);
    topLevelAction->setCheckable(true);
    connect(topLevelAction, SIGNAL(checked(bool)), SLOT(changeTopLevel(bool)));

    allowedAreasActions = new QActionGroup(this);
    allowedAreasActions->setExclusive(false);

    allowLeftAction = new QAction(tr("Allow on Left"), this);
    allowLeftAction->setCheckable(true);
    connect(allowLeftAction, SIGNAL(checked(bool)), SLOT(allowLeft(bool)));

    allowRightAction = new QAction(tr("Allow on Right"), this);
    allowRightAction->setCheckable(true);
    connect(allowRightAction, SIGNAL(checked(bool)), SLOT(allowRight(bool)));

    allowTopAction = new QAction(tr("Allow on Top"), this);
    allowTopAction->setCheckable(true);
    connect(allowTopAction, SIGNAL(checked(bool)), SLOT(allowTop(bool)));

    allowBottomAction = new QAction(tr("Allow on Bottom"), this);
    allowBottomAction->setCheckable(true);
    connect(allowBottomAction, SIGNAL(checked(bool)), SLOT(allowBottom(bool)));

    allowedAreasActions->addAction(allowLeftAction);
    allowedAreasActions->addAction(allowRightAction);
    allowedAreasActions->addAction(allowTopAction);
    allowedAreasActions->addAction(allowBottomAction);

    areaActions = new QActionGroup(this);
    areaActions->setExclusive(true);

    leftAction = new QAction(tr("Place on Left") , this);
    leftAction->setCheckable(true);
    connect(leftAction, SIGNAL(checked(bool)), SLOT(placeLeft(bool)));

    rightAction = new QAction(tr("Place on Right") , this);
    rightAction->setCheckable(true);
    connect(rightAction, SIGNAL(checked(bool)), SLOT(placeRight(bool)));

    topAction = new QAction(tr("Place on Top") , this);
    topAction->setCheckable(true);
    connect(topAction, SIGNAL(checked(bool)), SLOT(placeTop(bool)));

    bottomAction = new QAction(tr("Place on Bottom") , this);
    bottomAction->setCheckable(true);
    connect(bottomAction, SIGNAL(checked(bool)), SLOT(placeBottom(bool)));

    areaActions->addAction(leftAction);
    areaActions->addAction(rightAction);
    areaActions->addAction(topAction);
    areaActions->addAction(bottomAction);

    connect(movableAction, SIGNAL(checked(bool)), areaActions, SLOT(setEnabled(bool)));

    connect(movableAction, SIGNAL(checked(bool)), allowedAreasActions, SLOT(setEnabled(bool)));

    connect(floatableAction, SIGNAL(checked(bool)), topLevelAction, SLOT(setEnabled(bool)));

    connect(topLevelAction, SIGNAL(checked(bool)), floatableAction, SLOT(setDisabled(bool)));
    connect(movableAction, SIGNAL(checked(bool)), floatableAction, SLOT(setEnabled(bool)));

    menu = new QMenu(this);
    menu->addAction(showHideAction);
    menu->addSeparator();
    menu->addAction(closableAction);
    menu->addAction(movableAction);
    menu->addAction(floatableAction);
    menu->addAction(topLevelAction);
    menu->addSeparator();
    menu->addActions(allowedAreasActions->actions());
    menu->addSeparator();
    menu->addActions(areaActions->actions());
}

void ColorSwatch::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    menu->exec(event->globalPos());
}

void ColorSwatch::polishEvent(QEvent *)
{
    const Qt::DockWindowArea area = this->area();
    const Qt::DockWindowAreas areas = allowedAreas();

    closableAction->setChecked(hasFeature(QDockWindow::DockWindowClosable));
    if (testWFlags(Qt::WMacDrawer)) {
        floatableAction->setEnabled(false);
        topLevelAction->setEnabled(false);
        movableAction->setEnabled(false);
    } else {
        floatableAction->setChecked(hasFeature(QDockWindow::DockWindowFloatable));
        topLevelAction->setChecked(isTopLevel());
        // done after topLevel, to get 'floatable' correctly initialized
        movableAction->setChecked(hasFeature(QDockWindow::DockWindowMovable));
    }

    allowLeftAction->setChecked(isDockable(Qt::DockWindowAreaLeft));
    allowRightAction->setChecked(isDockable(Qt::DockWindowAreaRight));
    allowTopAction->setChecked(isDockable(Qt::DockWindowAreaTop));
    allowBottomAction->setChecked(isDockable(Qt::DockWindowAreaBottom));

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::DockWindowAreaLeft);
        allowRightAction->setEnabled(area != Qt::DockWindowAreaRight);
        allowTopAction->setEnabled(area != Qt::DockWindowAreaTop);
        allowBottomAction->setEnabled(area != Qt::DockWindowAreaBottom);
    }

    leftAction->setChecked(area == Qt::DockWindowAreaLeft);
    rightAction->setChecked(area == Qt::DockWindowAreaRight);
    topAction->setChecked(area == Qt::DockWindowAreaTop);
    bottomAction->setChecked(area == Qt::DockWindowAreaBottom);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::DockWindowAreaLeft);
        rightAction->setEnabled(areas & Qt::DockWindowAreaRight);
        topAction->setEnabled(areas & Qt::DockWindowAreaTop);
        bottomAction->setEnabled(areas & Qt::DockWindowAreaBottom);
    }
}

void ColorSwatch::hideEvent(QHideEvent *)
{
    showHideAction->setText(tr("Show"));
}

void ColorSwatch::showEvent(QShowEvent *)
{
    showHideAction->setText(tr("Hide"));
}

void ColorSwatch::showHide()
{ setShown(!isShown()); }

void ColorSwatch::allow(Qt::DockWindowArea area, bool a)
{
    Qt::DockWindowAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::DockWindowAreaLeft);
        rightAction->setEnabled(areas & Qt::DockWindowAreaRight);
        topAction->setEnabled(areas & Qt::DockWindowAreaTop);
        bottomAction->setEnabled(areas & Qt::DockWindowAreaBottom);
    }
}

void ColorSwatch::place(Qt::DockWindowArea area, bool p)
{
    if (!p) return;

    setArea(area);

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::DockWindowAreaLeft);
        allowRightAction->setEnabled(area != Qt::DockWindowAreaRight);
        allowTopAction->setEnabled(area != Qt::DockWindowAreaTop);
        allowBottomAction->setEnabled(area != Qt::DockWindowAreaBottom);
    }
}

void ColorSwatch::changeClosable(bool on)
{ setFeature(DockWindowClosable, on); }

void ColorSwatch::changeMovable(bool on)
{ setFeature(DockWindowMovable, on); }

void ColorSwatch::changeFloatable(bool on)
{ setFeature(DockWindowFloatable, on); }

void ColorSwatch::changeTopLevel(bool topLevel)
{
    setTopLevel(topLevel, mapToGlobal(QPoint(20, 20)));
}

void ColorSwatch::allowLeft(bool a)
{ allow(Qt::DockWindowAreaLeft, a); }

void ColorSwatch::allowRight(bool a)
{ allow(Qt::DockWindowAreaRight, a); }

void ColorSwatch::allowTop(bool a)
{ allow(Qt::DockWindowAreaTop, a); }

void ColorSwatch::allowBottom(bool a)
{ allow(Qt::DockWindowAreaBottom, a); }

void ColorSwatch::placeLeft(bool p)
{ place(Qt::DockWindowAreaLeft, p); }

void ColorSwatch::placeRight(bool p)
{ place(Qt::DockWindowAreaRight, p); }

void ColorSwatch::placeTop(bool p)
{ place(Qt::DockWindowAreaTop, p); }

void ColorSwatch::placeBottom(bool p)
{ place(Qt::DockWindowAreaBottom, p); }
