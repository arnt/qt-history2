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

#include "colorswatch.h"

#include <qaction.h>
#include <qevent.h>
#include <qframe.h>
#include <qmainwindow.h>
#include <qmenu.h>

ColorSwatch::ColorSwatch(const QString &colorName, QWidget *parent, Qt::WFlags flags)
    : QDockWindow(parent, flags)
{
    setObjectName(colorName + QLatin1String(" Dock Window"));
    setWindowTitle(objectName());

    QFrame *swatch = new QFrame(this);
    swatch->setFrameStyle(QFrame::Box | QFrame::Sunken);
    swatch->setMinimumSize(125, 75);

    QPalette pal = swatch->palette();
    pal.setColor(swatch->backgroundRole(), QColor(colorName));
    swatch->setPalette(pal);

    setWidget(swatch);

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

    menu = new QMenu(colorName, this);
    menu->addAction(toggleViewAction());
    menu->addSeparator();
    menu->addAction(closableAction);
    menu->addAction(movableAction);
    menu->addAction(floatableAction);
    menu->addAction(topLevelAction);
    menu->addSeparator();
    menu->addActions(allowedAreasActions->actions());
    menu->addSeparator();
    menu->addActions(areaActions->actions());

    if(colorName == "Black") {
        leftAction->setShortcut(Qt::CTRL|Qt::Key_W);
        rightAction->setShortcut(Qt::CTRL|Qt::Key_E);
        toggleViewAction()->setShortcut(Qt::CTRL|Qt::Key_R);
    }
}

void ColorSwatch::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    menu->exec(event->globalPos());
}

void ColorSwatch::polishEvent(QEvent *)
{
    QMainWindow *mainWindow = qt_cast<QMainWindow *>(parentWidget());
    const Qt::DockWindowArea area = mainWindow->dockWindowArea(this);
    const Qt::DockWindowAreas areas = allowedAreas();

    closableAction->setChecked(features() & QDockWindow::DockWindowClosable);
    if (windowType() == Qt::Drawer) {
        floatableAction->setEnabled(false);
        topLevelAction->setEnabled(false);
        movableAction->setEnabled(false);
    } else {
        floatableAction->setChecked(features() & QDockWindow::DockWindowFloatable);
        topLevelAction->setChecked(isWindow());
        // done after topLevel, to get 'floatable' correctly initialized
        movableAction->setChecked(features() & QDockWindow::DockWindowMovable);
    }

    allowLeftAction->setChecked(isAreaAllowed(Qt::LeftDockWindowArea));
    allowRightAction->setChecked(isAreaAllowed(Qt::RightDockWindowArea));
    allowTopAction->setChecked(isAreaAllowed(Qt::TopDockWindowArea));
    allowBottomAction->setChecked(isAreaAllowed(Qt::BottomDockWindowArea));

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::LeftDockWindowArea);
        allowRightAction->setEnabled(area != Qt::RightDockWindowArea);
        allowTopAction->setEnabled(area != Qt::TopDockWindowArea);
        allowBottomAction->setEnabled(area != Qt::BottomDockWindowArea);
    }

    leftAction->blockSignals(true);
    rightAction->blockSignals(true);
    topAction->blockSignals(true);
    bottomAction->blockSignals(true);

    leftAction->setChecked(area == Qt::LeftDockWindowArea);
    rightAction->setChecked(area == Qt::RightDockWindowArea);
    topAction->setChecked(area == Qt::TopDockWindowArea);
    bottomAction->setChecked(area == Qt::BottomDockWindowArea);

    leftAction->blockSignals(false);
    rightAction->blockSignals(false);
    topAction->blockSignals(false);
    bottomAction->blockSignals(false);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::LeftDockWindowArea);
        rightAction->setEnabled(areas & Qt::RightDockWindowArea);
        topAction->setEnabled(areas & Qt::TopDockWindowArea);
        bottomAction->setEnabled(areas & Qt::BottomDockWindowArea);
    }
}

void ColorSwatch::allow(Qt::DockWindowArea area, bool a)
{
    Qt::DockWindowAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::LeftDockWindowArea);
        rightAction->setEnabled(areas & Qt::RightDockWindowArea);
        topAction->setEnabled(areas & Qt::TopDockWindowArea);
        bottomAction->setEnabled(areas & Qt::BottomDockWindowArea);
    }
}

void ColorSwatch::place(Qt::DockWindowArea area, bool p)
{
    if (!p) return;

    QMainWindow *mainWindow = qt_cast<QMainWindow *>(parentWidget());
    mainWindow->addDockWindow(area, this);

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::LeftDockWindowArea);
        allowRightAction->setEnabled(area != Qt::RightDockWindowArea);
        allowTopAction->setEnabled(area != Qt::TopDockWindowArea);
        allowBottomAction->setEnabled(area != Qt::BottomDockWindowArea);
    }
}

void ColorSwatch::changeClosable(bool on)
{ setFeatures(on ? features() | DockWindowClosable : features() & ~DockWindowClosable); }

void ColorSwatch::changeMovable(bool on)
{ setFeatures(on ? features() | DockWindowMovable : features() & ~DockWindowMovable); }

void ColorSwatch::changeFloatable(bool on)
{ setFeatures(on ? features() | DockWindowFloatable : features() & ~DockWindowFloatable); }

void ColorSwatch::changeTopLevel(bool topLevel)
{ setTopLevel(topLevel); }

void ColorSwatch::allowLeft(bool a)
{ allow(Qt::LeftDockWindowArea, a); }

void ColorSwatch::allowRight(bool a)
{ allow(Qt::RightDockWindowArea, a); }

void ColorSwatch::allowTop(bool a)
{ allow(Qt::TopDockWindowArea, a); }

void ColorSwatch::allowBottom(bool a)
{ allow(Qt::BottomDockWindowArea, a); }

void ColorSwatch::placeLeft(bool p)
{ place(Qt::LeftDockWindowArea, p); }

void ColorSwatch::placeRight(bool p)
{ place(Qt::RightDockWindowArea, p); }

void ColorSwatch::placeTop(bool p)
{ place(Qt::TopDockWindowArea, p); }

void ColorSwatch::placeBottom(bool p)
{ place(Qt::BottomDockWindowArea, p); }
