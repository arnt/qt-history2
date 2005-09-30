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

#include "dropsitewindow.h"

DropSiteWindow::DropSiteWindow(QWidget *parent)
    : QWidget(parent)
{
    abstractLabel = new QLabel(tr("The Drop Site example accepts drops from other "
                                  "applications, and displays the MIME formats "
                                  "provided by the drag object."));
    abstractLabel->setWordWrap(true);
    abstractLabel->adjustSize();

    dropSiteWidget = new DropSiteWidget;
    connect(dropSiteWidget, SIGNAL(changed(const QMimeData*)),
            this, SLOT(updateSupportedFormats(const QMimeData*)));

    supportedFormats = new QTableWidget(0, 2);
    QStringList labels;
    labels << tr("Format") << tr("Content");
    supportedFormats->setHorizontalHeaderLabels(labels);
    supportedFormats->horizontalHeader()->setStretchLastSection(true);

    quitButton = new QPushButton(tr("Quit"));
    connect(quitButton, SIGNAL(pressed()), this, SLOT(close()));

    clearButton = new QPushButton(tr("Clear"));
    connect(clearButton, SIGNAL(pressed()), dropSiteWidget, SLOT(clear()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch();

    layout = new QVBoxLayout;
    layout->addWidget(abstractLabel);
    layout->addWidget(dropSiteWidget);
    layout->addWidget(supportedFormats);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setMinimumSize(350, 500);
    setWindowTitle(tr("Drop Site"));
}

void DropSiteWindow::resizeEvent(QResizeEvent *event)
{
    supportedFormats->resizeColumnToContents(0);
    QWidget::resizeEvent(event);
}

void DropSiteWindow::updateSupportedFormats(const QMimeData *mimeData)
{
    supportedFormats->setRowCount(0);

    if (!mimeData)
        return;

    QStringList formats = mimeData->formats();

    foreach (QString format, formats)
    {
        QTableWidgetItem *formatItem = new QTableWidgetItem(format);
        formatItem->setFlags(Qt::ItemIsEnabled);
        formatItem->setTextAlignment(Qt::AlignTop);

        QByteArray data = mimeData->data(format);
        QTableWidgetItem *dataItem;

        QString text = dropSiteWidget->createPlainText(data, format);
        qApp->processEvents();
        if (!text.isEmpty()) {
            dataItem = new QTableWidgetItem(text);
        } else {
            QString hexdata = "";
            foreach (uint byte, data) {
                if (hexdata.length() < 128) {
                    QString hex = QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
                    hexdata.append(hex + " ");
                }
            }
            dataItem = new QTableWidgetItem(hexdata);
        }
        dataItem->setFlags(Qt::ItemIsEnabled);

        int row = supportedFormats->rowCount();
        supportedFormats->insertRow(row);
        supportedFormats->setItem(row, 0, formatItem);
        supportedFormats->setItem(row, 1, dataItem);

        QTableWidgetItem *index = new QTableWidgetItem(tr("%1").arg(row + 1));
        supportedFormats->setVerticalHeaderItem(row, index);
    }

    supportedFormats->resizeColumnToContents(0);
}
