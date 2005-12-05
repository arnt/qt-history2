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

#include "findfiledialog.h"

FindFileDialog::FindFileDialog(QTextEdit *editor, QAssistantClient *assistant,
                       QWidget *parent)
    : QDialog(parent)
{
    currentAssistantClient = assistant;
    currentEditor = editor;

    createButtons();
    createComboBoxes();
    createLabels();
    createFilesTable();
    createLayout();

    directoryComboBox->addItem(QDir::currentPath() + QDir::separator());
    fileNameComboBox->addItem("*");
    findFiles();

    setWindowTitle(tr("Find File"));
}

void FindFileDialog::browse()
{
    QString currentDirectory = directoryComboBox->currentText();
    QString newDirectory = QFileDialog::getExistingDirectory(this,
                               tr("Select Directory"), currentDirectory);
    if (!newDirectory.isEmpty()) {
        directoryComboBox->addItem(newDirectory);
        directoryComboBox->setCurrentIndex(directoryComboBox->count() - 1);
        update();
    }
}

void FindFileDialog::help()
{
//  currentAssistantClient->showPage(QLibraryInfo::location(QLibraryInfo::ExamplesPath) +
//                                   "assistant/simpletextviewer/documentation/filedialog.html");
    currentAssistantClient->showPage("/home/vkarlsen/dev/research/newexamples/assistant/simpletextviewer/documentation/filedialog.html");
}

void FindFileDialog::openFile(int row, int column)
{
    QTableWidgetItem *item = filesFoundTable->item(row, column);
    if (!item)
        item = filesFoundTable->currentItem();

    QString fileName = item->text();

    if (!fileName.isEmpty()) {
        QString path = directoryComboBox->currentText();
        QFile file(path + fileName);

        if (file.open(QIODevice::ReadOnly)) {
            QString data(file.readAll());

            if (fileName.endsWith(".html"))
                currentEditor->setHtml(data);
            else
                currentEditor->setPlainText(data);
        }
    }
    close();
}

void FindFileDialog::update()
{
    findFiles();
    openButton->setEnabled(!(filesFoundTable->rowCount() == 0));
}

void FindFileDialog::select(QTableWidgetItem *current,
                        QTableWidgetItem *previous)
{
    openButton->setEnabled(true);

    current->setBackgroundColor(palette().alternateBase().color());
    if (previous)
        previous->setBackgroundColor(palette().base().color());
}

void FindFileDialog::findFiles()
{
    QRegExp filePattern(fileNameComboBox->currentText() + "*");
    filePattern.setPatternSyntax(QRegExp::Wildcard);

    QDir directory(directoryComboBox->currentText());

    QStringList allFiles = directory.entryList(QDir::Files | QDir::NoSymLinks);
    QStringList matchingFiles;

    foreach (QString file, allFiles) {
        if (filePattern.exactMatch(file))
            matchingFiles << file;
    }
    showFiles(matchingFiles);
}

void FindFileDialog::showFiles(const QStringList &files)
{
    filesFoundTable->setRowCount(0);

    for (int i = 0; i < files.count(); ++i) {
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(files[i]);
        fileNameItem->setFlags(Qt::ItemIsEnabled);

        int row = filesFoundTable->rowCount();
        filesFoundTable->insertRow(row);
        filesFoundTable->setItem(row, 0, fileNameItem);
    }

    if (files.count() > 0)
        filesFoundTable->setCurrentItem(filesFoundTable->item(0,0));
}

void FindFileDialog::createButtons()
{
    cancelButton = new QPushButton(tr("&Cancel"));
    openButton = new QPushButton(tr("&Open"));
    helpButton = new QPushButton(tr("&Help"));

    browseButton = new QToolButton();
    browseButton->setText("...");

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(helpButton, SIGNAL(clicked()), this, SLOT(help()));

    openButton->setDefault(true);
    openButton->setEnabled(false);
}

void FindFileDialog::createComboBoxes()
{
    directoryComboBox = new QComboBox;
    fileNameComboBox = new QComboBox;

    fileNameComboBox->setEditable(true);
    fileNameComboBox->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Preferred);

    directoryComboBox->setMinimumContentsLength(30);
    directoryComboBox->setSizeAdjustPolicy(
            QComboBox::AdjustToMinimumContentsLength);
    directoryComboBox->setSizePolicy(QSizePolicy::Expanding,
                                     QSizePolicy::Preferred);


    connect(fileNameComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(update()));
    connect(directoryComboBox, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(update()));
}

void FindFileDialog::createFilesTable()
{
    filesFoundTable = new QTableWidget(0, 1);
    QStringList labels;
    labels << tr("Matching Files");
    filesFoundTable->setHorizontalHeaderLabels(labels);
    filesFoundTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    filesFoundTable->verticalHeader()->hide();
    filesFoundTable->setShowGrid(false);

    connect(filesFoundTable, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(openFile(int, int)));
    connect(filesFoundTable,
            SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),
            this,
            SLOT(select(QTableWidgetItem *, QTableWidgetItem *)));
}

void FindFileDialog::createLabels()
{
    directoryLabel = new QLabel(tr("Search in:"));
    fileNameLabel = new QLabel(tr("File name (including wildcards):"));
}

void FindFileDialog::createLayout()
{
    QHBoxLayout *fileLayout = new QHBoxLayout;
    fileLayout->addWidget(fileNameLabel);
    fileLayout->addWidget(fileNameComboBox);

    QHBoxLayout *directoryLayout = new QHBoxLayout;
    directoryLayout->addWidget(directoryLabel);
    directoryLayout->addWidget(directoryComboBox);
    directoryLayout->addWidget(browseButton);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(helpButton);
    buttons->addStretch();
    buttons->addWidget(openButton);
    buttons->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(fileLayout);
    mainLayout->addLayout(directoryLayout);
    mainLayout->addWidget(filesFoundTable);
    mainLayout->addStretch();
    mainLayout->addLayout(buttons);
    setLayout(mainLayout);
}
