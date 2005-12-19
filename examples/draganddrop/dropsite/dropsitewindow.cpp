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

    QStringList labels;
    labels << tr("Format") << tr("Content");

    formatsTable = new QTableWidget;
    formatsTable->setColumnCount(2);
    formatsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

    foreach (QString format, mimeData->formats()) {
        QByteArray data = mimeData->data(format);
        QString text;

        qApp->processEvents();

        if (format.startsWith("text/")) {
            text = dropArea->extractText(data, format).simplified();
        } else {
            for (int i = 0; i < data.size() && i < 32; ++i) {
                QString hex = QString("%1").arg(uchar(data[i]), 2, 16,
                                                QChar('0'))
                                           .toUpper();
                text.append(hex + " ");
            }
        }

        int row = formatsTable->rowCount();
        formatsTable->insertRow(row);
        formatsTable->setItem(row, 0, new QTableWidgetItem(format));
        formatsTable->setItem(row, 1, new QTableWidgetItem(text));

        QTableWidgetItem *index = new QTableWidgetItem(tr("%1").arg(row + 1));
        formatsTable->setVerticalHeaderItem(row, index);
    }

    formatsTable->resizeColumnToContents(0);
}
