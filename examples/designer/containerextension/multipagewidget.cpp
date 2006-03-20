/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "multipagewidget.h"

MultiPageWidget::MultiPageWidget(QWidget *parent)
    : QWidget(parent)
{
    comboBox = new QComboBox();
    comboBox->setObjectName("__qt__passive_comboBox");
    stackWidget = new QStackedWidget();

    connect(comboBox, SIGNAL(activated(int)),
            stackWidget, SLOT(setCurrentIndex(int)));

    layout = new QVBoxLayout();
    layout->addWidget(comboBox);
    layout->addWidget(stackWidget);
    setLayout(layout);
}

QSize MultiPageWidget::sizeHint() const
{
    return QSize(200, 150);
}

void MultiPageWidget::addPage(QWidget *page)
{
    page->setParent(stackWidget);

    stackWidget->addWidget(page);
    comboBox->addItem(tr("Page %1").arg(comboBox->count() + 1));

    stackWidget->setCurrentIndex(comboBox->count() -1 );
    comboBox->setCurrentIndex(comboBox->count() -1 );
}

void MultiPageWidget::removePage(int index)
{
    QWidget *widget = stackWidget->widget(index);
    stackWidget->removeWidget(widget);

    comboBox->removeItem(index);
}

int MultiPageWidget::count()
{
    return stackWidget->count();
}

int MultiPageWidget::currentIndex()
{
    return stackWidget->currentIndex();
}

void MultiPageWidget::insertPage(int index, QWidget *page)
{
    page->setParent(stackWidget);

    stackWidget->insertWidget(index, page);
    comboBox->insertItem(index, tr("Page %1").arg(comboBox->count() + 1));

    stackWidget->setCurrentIndex(index );
    comboBox->setCurrentIndex(index );
}

void MultiPageWidget::setCurrentIndex(int index)
{
    stackWidget->setCurrentIndex(index);
    comboBox->setCurrentIndex(index);
}

QWidget* MultiPageWidget::widget(int index)
{
    return stackWidget->widget(index);
}

QString MultiPageWidget::pageTitle() const
{
    return comboBox->currentText();
}

void MultiPageWidget::setPageTitle(QString const &newTitle)
{
    comboBox->setItemText(currentIndex(), newTitle);
}
