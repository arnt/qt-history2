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

#include "toolbar.h"

#include <qmenu.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <stdlib.h>

static QPixmap genIcon(const QString &string, const QColor &color)
{
    QFont font;
    font.setBold(true);
    font.setPixelSize(16);

    QFontMetrics fm(font);
    int w = qMax(22, fm.width(string));
    int h = qMax(22, fm.lineSpacing());

    QPixmap pixmap(w, h);
    pixmap.fill(Qt::white);

    QPainter p(&pixmap);
    p.setPen(Qt::black);
    p.setFont(font);
    p.drawText(0, 0, w, h, Qt::AlignCenter, string);
    p.end();

    QImage image = pixmap;
    image.setAlphaBuffer(true);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QRgb rgba = image.pixel(x, y);
            const int alpha = qMin(255, qMax(0, (qGray(rgba)+32)/64*64));
            image.setPixel(x, y, qRgba(color.red(), color.green(), color.blue(), 255-alpha));
        }
    }
    pixmap = image;

    return pixmap;
}

static QPixmap genIcon(int number, const QColor &color)
{ return genIcon(QString::number(number), color); }

ToolBar::ToolBar(QMainWindow *parent)
    : QToolBar(parent), spinbox(0), spinboxAction(0)
{
    menu = new QMenu("One", this);
    menu->setIcon(genIcon(1, Qt::white));
    menu->addAction(genIcon("1.1", Qt::yellow), "One One");
    menu->addAction(genIcon("1.2", Qt::yellow), "One Two");
    menu->addAction(genIcon("1.3", Qt::yellow), "One Three");

    QAction *two = addAction(genIcon(2, QColor(0, 0, 160)), "Two");
    QFont boldFont;
    boldFont.setBold(true);
    two->setFont(boldFont);

    addAction(genIcon(3, QColor(0, 160, 0)), "Three");
    addAction(genIcon(4, QColor(160, 0, 0)), "Four");
    addAction(genIcon(5, QColor(80, 0, 80)), "Five");

    QAction *six = addAction(genIcon(6, QColor(0, 80, 80)), "Six");
    six->setCheckable(true);
    QAction *seven = addAction(genIcon(7, QColor(80, 80, 0)), "Seven");
    seven->setCheckable(true);
    QAction *eight = addAction(genIcon(8, Qt::black), "Eight");
    eight->setCheckable(true);

    showHideAction = new QAction(this);
    showHideAction->setText(tr("Hide"));
    connect(showHideAction, SIGNAL(triggered()), SLOT(showHide()));

    orderAction = new QAction(this);
    orderAction->setText(tr("Order Items in Tool Bar"));
    connect(orderAction, SIGNAL(triggered()), SLOT(order()));

    randomizeAction = new QAction(this);
    randomizeAction->setText(tr("Randomize Items in Tool Bar"));
    connect(randomizeAction, SIGNAL(triggered()), SLOT(randomize()));

    addSpinBoxAction = new QAction(this);
    addSpinBoxAction->setText(tr("Add Spin Box"));
    connect(addSpinBoxAction, SIGNAL(triggered()), SLOT(addSpinBox()));

    removeSpinBoxAction = new QAction(this);
    removeSpinBoxAction->setText(tr("Remove Spin Box"));
    removeSpinBoxAction->setEnabled(false);
    connect(removeSpinBoxAction, SIGNAL(triggered()), SLOT(removeSpinBox()));

    movableAction = new QAction(tr("Movable"), this);
    movableAction->setCheckable(true);
    connect(movableAction, SIGNAL(checked(bool)), SLOT(changeMovable(bool)));

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

    menu = new QMenu(tr("Tool Bar"), this);
    menu->addAction(showHideAction);
    menu->addSeparator();
    menu->addAction(orderAction);
    menu->addAction(randomizeAction);
    menu->addSeparator();
    menu->addAction(addSpinBoxAction);
    menu->addAction(removeSpinBoxAction);
    menu->addSeparator();
    menu->addAction(movableAction);
    menu->addSeparator();
    menu->addActions(allowedAreasActions->actions());
    menu->addSeparator();
    menu->addActions(areaActions->actions());

    randomize();
}


void ToolBar::polishEvent(QEvent *)
{
    const Qt::ToolBarArea area = this->area();
    const Qt::ToolBarAreas areas = allowedAreas();

    movableAction->setChecked(isMovable());

    allowLeftAction->setChecked(isDockable(Qt::ToolBarAreaLeft));
    allowRightAction->setChecked(isDockable(Qt::ToolBarAreaRight));
    allowTopAction->setChecked(isDockable(Qt::ToolBarAreaTop));
    allowBottomAction->setChecked(isDockable(Qt::ToolBarAreaBottom));

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::ToolBarAreaLeft);
        allowRightAction->setEnabled(area != Qt::ToolBarAreaRight);
        allowTopAction->setEnabled(area != Qt::ToolBarAreaTop);
        allowBottomAction->setEnabled(area != Qt::ToolBarAreaBottom);
    }

    leftAction->setChecked(area == Qt::ToolBarAreaLeft);
    rightAction->setChecked(area == Qt::ToolBarAreaRight);
    topAction->setChecked(area == Qt::ToolBarAreaTop);
    bottomAction->setChecked(area == Qt::ToolBarAreaBottom);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::ToolBarAreaLeft);
        rightAction->setEnabled(areas & Qt::ToolBarAreaRight);
        topAction->setEnabled(areas & Qt::ToolBarAreaTop);
        bottomAction->setEnabled(areas & Qt::ToolBarAreaBottom);
    }
}

void ToolBar::hideEvent(QHideEvent *)
{
    showHideAction->setText(tr("Show"));
}

void ToolBar::showEvent(QShowEvent *)
{
    showHideAction->setText(tr("Hide"));
}

void ToolBar::showHide()
{ setShown(!isShown()); }

void ToolBar::order()
{
    QList<QAction *> ordered, actions1 = actions(),
                              actions2 = qFindChildren<QAction *>(this);
    while (!actions2.isEmpty()) {
        QAction *action = actions2.takeFirst();
        if (!actions1.contains(action))
            continue;
        actions1.removeAll(action);
        ordered.append(action);
    }

    clear();
    addActions(ordered);

    orderAction->setEnabled(false);
}

void ToolBar::randomize()
{
    QList<QAction *> randomized, actions = this->actions();
    while (!actions.isEmpty()) {
        QAction *action = actions.takeAt(rand() % actions.size());
        randomized.append(action);
    }
    clear();
    addActions(randomized);

    orderAction->setEnabled(true);
}

void ToolBar::addSpinBox()
{
    if (!spinbox) {
        spinbox = new QSpinBox(this);
    }
    if (!spinboxAction)
        spinboxAction = addWidget(spinbox);
    else
        addAction(spinboxAction);

    addSpinBoxAction->setEnabled(false);
    removeSpinBoxAction->setEnabled(true);
}

void ToolBar::removeSpinBox()
{
    if (spinboxAction)
        removeAction(spinboxAction);

    addSpinBoxAction->setEnabled(true);
    removeSpinBoxAction->setEnabled(false);
}

void ToolBar::allow(Qt::ToolBarArea area, bool a)
{
    Qt::ToolBarAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);

    if (areaActions->isEnabled()) {
        leftAction->setEnabled(areas & Qt::ToolBarAreaLeft);
        rightAction->setEnabled(areas & Qt::ToolBarAreaRight);
        topAction->setEnabled(areas & Qt::ToolBarAreaTop);
        bottomAction->setEnabled(areas & Qt::ToolBarAreaBottom);
    }
}

void ToolBar::place(Qt::ToolBarArea area, bool p)
{
    if (!p) return;

    setArea(area);

    if (allowedAreasActions->isEnabled()) {
        allowLeftAction->setEnabled(area != Qt::ToolBarAreaLeft);
        allowRightAction->setEnabled(area != Qt::ToolBarAreaRight);
        allowTopAction->setEnabled(area != Qt::ToolBarAreaTop);
        allowBottomAction->setEnabled(area != Qt::ToolBarAreaBottom);
    }
}

void ToolBar::changeMovable(bool movable)
{ setMovable(movable); }

void ToolBar::allowLeft(bool a)
{ allow(Qt::ToolBarAreaLeft, a); }

void ToolBar::allowRight(bool a)
{ allow(Qt::ToolBarAreaRight, a); }

void ToolBar::allowTop(bool a)
{ allow(Qt::ToolBarAreaTop, a); }

void ToolBar::allowBottom(bool a)
{ allow(Qt::ToolBarAreaBottom, a); }

void ToolBar::placeLeft(bool p)
{ place(Qt::ToolBarAreaLeft, p); }

void ToolBar::placeRight(bool p)
{ place(Qt::ToolBarAreaRight, p); }

void ToolBar::placeTop(bool p)
{ place(Qt::ToolBarAreaTop, p); }

void ToolBar::placeBottom(bool p)
{ place(Qt::ToolBarAreaBottom, p); }
