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

#include "droparea.h"
#include "dropsitewindow.h"

DropSiteWindow::DropSiteWindow()
{
    abstractLabel = new QLabel(tr("This example accepts drags from other "
                                  "applications and displays the MIME types "
                                  "provided by the drag object."));
    abstractLabel->setWordWrap(true);
    abstractLabel->adjustSize();

    dropArea = new DropArea;
    connect(dropArea, SIGNAL(changed(const QMimeData *)),
            this, SLOT(updateFormatsTable(const QMimeData *)));

    formatsTable = new QTableWidget(0, 2);
    QStringList labels;
    labels << tr("Format") << tr("Content");
    formatsTable->setHorizontalHeaderLabels(labels);
    formatsTable->horizontalHeader()->setStretchLastSection(true);

    quitButton = new QPushButton(tr("Quit"));
    connect(quitButton, SIGNAL(pressed()), this, SLOT(close()));

    clearButton = new QPushButton(tr("Clear"));
    connect(clearButton, SIGNAL(pressed()), dropArea, SLOT(clear()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(abstractLabel);
    mainLayout->addWidget(dropArea);
    mainLayout->addWidget(formatsTable);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setMinimumSize(350, 500);
    setWindowTitle(tr("Drop Site"));
}

void DropSiteWindow::updateFormatsTable(const QMimeData *mimeData)
{
    formatsTable->setRowCount(0);

    if (!mimeData)
        return;

    QStringList formats = mimeData->formats();

    foreach (QString format, formats) {
        QTableWidgetItem *formatItem = new QTableWidgetItem(format);
        formatItem->setFlags(Qt::ItemIsEnabled);
        formatItem->setTextAlignment(Qt::AlignTop);

        QByteArray data = mimeData->data(format);
        QString text;

        qApp->processEvents();

        if (format.startsWith("text/")) {
            text = dropArea->extractText(data, format).simplified();
        } else {
            for (int i = 0; i < data.size() && i < 32; ++i) {
                QString hex = QString("%1").arg(data[i], 2, 16, QChar('0'))
                                           .toUpper();
                text.append(hex + " ");
            }
        }

        QTableWidgetItem *dataItem = new QTableWidgetItem(text);
        dataItem->setFlags(Qt::ItemIsEnabled);

        int row = formatsTable->rowCount();
        formatsTable->insertRow(row);
        formatsTable->setItem(row, 0, formatItem);
        formatsTable->setItem(row, 1, dataItem);

        QTableWidgetItem *index = new QTableWidgetItem(tr("%1").arg(row + 1));
        formatsTable->setVerticalHeaderItem(row, index);
    }

    formatsTable->resizeColumnToContents(0);
}
