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

#include "abstractsettingsdialog.h"

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QToolBar>

AbstractSettingsDialog::AbstractSettingsDialog(QWidget *parent)
    : QMainWindow(parent, Qt::Dialog)
{
    setWindowTitle(tr("Designer Options..."));

    toolbar = addToolBar(tr("Settings"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(48, 48));

    g = new QActionGroup(this);
    g->setExclusive(true);

    connect(g, SIGNAL(triggered(QAction*)), this, SLOT(showSettings(QAction*)));
}

void AbstractSettingsDialog::addPage(QWidget *widget, const QIcon &icon)
{
    QAction *a = g->addAction(QString());
    toolbar->addAction(a);
    a->setIcon(icon);
    g->addAction(a);
    widget->setParent(this);
    widget->hide();
    pages[a] = widget;

    if (centralWidget() == 0)
        showSettings(a);
}

void AbstractSettingsDialog::showSettings(QAction *a)
{
    QWidget *oldPage = centralWidget();

    QWidget *page = pages.value(a);
    Q_ASSERT(page != 0);

    if (oldPage == page)
        return;

    if (oldPage != 0) {
        oldPage->hide();
        setCentralWidget(0);
    }

    setCentralWidget(page);
    page->show();

    adjustSize();
}

