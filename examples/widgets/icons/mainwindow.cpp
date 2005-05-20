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

#include "iconpreviewarea.h"
#include "iconsizespinbox.h"
#include "imagedelegate.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    centralWidget = new QWidget;
    setCentralWidget(centralWidget);

    createPreviewGroupBox();
    createImagesGroupBox();
    createIconSizeGroupBox();

    createActions();
    createMenus();
    createContextMenu();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(imagesGroupBox, 0, 0);
    mainLayout->addWidget(iconSizeGroupBox, 1, 0);
    mainLayout->addWidget(previewGroupBox, 0, 1, 2, 1);
    centralWidget->setLayout(mainLayout);

    setWindowTitle(tr("Icons"));
    checkCurrentStyle();
    otherRadioButton->click();

    resize(860, 400);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Icons"),
            tr("The <b>Icons</b> example illustrates how Qt renders an icon in "
               "different modes (active, normal, and disabled) and states (on "
               "and off) based on a set of images."));
}

void MainWindow::changeStyle(bool checked)
{
    if (!checked)
        return;

    QAction *action = qobject_cast<QAction *>(sender());
    QStyle *style = QStyleFactory::create(action->data().toString());
    Q_ASSERT(style);
    QApplication::setStyle(style);

    smallRadioButton->setText(tr("Small (%1 × %1)")
            .arg(style->pixelMetric(QStyle::PM_SmallIconSize)));
    largeRadioButton->setText(tr("Large (%1 × %1)")
            .arg(style->pixelMetric(QStyle::PM_LargeIconSize)));
    toolBarRadioButton->setText(tr("Toolbars (%1 × %1)")
            .arg(style->pixelMetric(QStyle::PM_ToolBarIconSize)));
    listViewRadioButton->setText(tr("List views (%1 × %1)")
            .arg(style->pixelMetric(QStyle::PM_ListViewIconSize)));
    iconViewRadioButton->setText(tr("Icon views (%1 × %1)")
            .arg(style->pixelMetric(QStyle::PM_IconViewIconSize)));

    changeSize();
}

void MainWindow::changeSize()
{
    int extent;

    if (otherRadioButton->isChecked()) {
        extent = otherSpinBox->value();
    } else {
        QStyle::PixelMetric metric;

        if (smallRadioButton->isChecked()) {
            metric = QStyle::PM_SmallIconSize;
        } else if (largeRadioButton->isChecked()) {
            metric = QStyle::PM_LargeIconSize;
        } else if (toolBarRadioButton->isChecked()) {
            metric = QStyle::PM_ToolBarIconSize;
        } else if (listViewRadioButton->isChecked()) {
            metric = QStyle::PM_ListViewIconSize;
        } else {
            metric = QStyle::PM_IconViewIconSize;
        }
        extent = QApplication::style()->pixelMetric(metric);
    }
    previewArea->setSize(QSize(extent, extent));
    otherSpinBox->setEnabled(otherRadioButton->isChecked());
}

void MainWindow::changeIcon()
{
    QIcon icon;

    for (int row = 0; row < imagesTable->rowCount(); ++row) {
        QTableWidgetItem *item0 = imagesTable->item(row, 0);
        QTableWidgetItem *item1 = imagesTable->item(row, 1);
        QTableWidgetItem *item2 = imagesTable->item(row, 2);

        if (item0->checkState() == Qt::Checked) {
            QIcon::Mode mode;
            if (item1->text() == tr("Normal")) {
                mode = QIcon::Normal;
            } else if (item1->text() == tr("Active")) {
                mode = QIcon::Active;
            } else {
                mode = QIcon::Disabled;
            }

            QIcon::State state;
            if (item2->text() == tr("On")) {
                state = QIcon::On;
            } else {
                state = QIcon::Off;
            }

            QString fileName = item0->data(Qt::UserRole).toString();
            QImage image(fileName);
            if (!image.isNull())
                icon.addPixmap(QPixmap::fromImage(image), mode, state);
        }
    }

    previewArea->setIcon(icon);
}

void MainWindow::addImage()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                    tr("Open Images"), "",
                                    tr("Images (*.png *.xpm *.jpg);;"
                                       "All Files (*)"));
    if (!fileNames.isEmpty()) {
        foreach (QString fileName, fileNames) {
            int row = imagesTable->rowCount();
            imagesTable->setRowCount(row + 1);

            QString imageName = QFileInfo(fileName).baseName();
            QTableWidgetItem *item0 = new QTableWidgetItem(imageName);
            item0->setData(Qt::UserRole, fileName);
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);

            QTableWidgetItem *item1 = new QTableWidgetItem(tr("Normal"));
            QTableWidgetItem *item2 = new QTableWidgetItem(tr("Off"));

            if (guessModeStateAct->isChecked()) {
                if (fileName.contains("_act")) {
                    item1->setText(tr("Active"));
                } else if (fileName.contains("_dis")) {
                    item1->setText(tr("Disabled"));
                }

                if (fileName.contains("_on"))
                    item2->setText(tr("On"));
            }

            imagesTable->setItem(row, 0, item0);
            imagesTable->setItem(row, 1, item1);
            imagesTable->setItem(row, 2, item2);
            imagesTable->openPersistentEditor(item1);
            imagesTable->openPersistentEditor(item2);

            item0->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::removeAllImages()
{
    imagesTable->setRowCount(0);
    changeIcon();
}

void MainWindow::createPreviewGroupBox()
{
    previewGroupBox = new QGroupBox(tr("Preview"));

    previewArea = new IconPreviewArea;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(previewArea);
    previewGroupBox->setLayout(layout);
}

void MainWindow::createImagesGroupBox()
{
    imagesGroupBox = new QGroupBox(tr("Images"));
    imagesGroupBox->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);

    QStringList labels;
    labels << tr("Image") << tr("Mode") << tr("State");

    imagesTable = new QTableWidget;
    imagesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    imagesTable->setSelectionMode(QAbstractItemView::NoSelection);
    imagesTable->setColumnCount(3);
    imagesTable->setHorizontalHeaderLabels(labels);
    imagesTable->setItemDelegate(new ImageDelegate(this));

    imagesTable->horizontalHeader()->resizeSection(0, 160);
    imagesTable->horizontalHeader()->resizeSection(1, 80);
    imagesTable->horizontalHeader()->resizeSection(2, 80);
    imagesTable->verticalHeader()->hide();

    connect(imagesTable, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(changeIcon()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imagesTable);
    imagesGroupBox->setLayout(layout);
}

void MainWindow::createIconSizeGroupBox()
{
    iconSizeGroupBox = new QGroupBox(tr("Icon Size"));

    smallRadioButton = new QRadioButton;
    largeRadioButton = new QRadioButton;
    toolBarRadioButton = new QRadioButton;
    listViewRadioButton = new QRadioButton;
    iconViewRadioButton = new QRadioButton;
    otherRadioButton = new QRadioButton(tr("Other:"));

    otherSpinBox = new IconSizeSpinBox;
    otherSpinBox->setRange(8, 128);
    otherSpinBox->setValue(64);

    connect(toolBarRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(changeSize()));
    connect(listViewRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(changeSize()));
    connect(iconViewRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(changeSize()));
    connect(smallRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeSize()));
    connect(largeRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeSize()));
    connect(otherRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeSize()));
    connect(otherSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeSize()));

    QHBoxLayout *otherSizeLayout = new QHBoxLayout;
    otherSizeLayout->addWidget(otherRadioButton);
    otherSizeLayout->addWidget(otherSpinBox);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(smallRadioButton, 0, 0);
    layout->addWidget(largeRadioButton, 1, 0);
    layout->addWidget(toolBarRadioButton, 2, 0);
    layout->addWidget(listViewRadioButton, 0, 1);
    layout->addWidget(iconViewRadioButton, 1, 1);
    layout->addLayout(otherSizeLayout, 2, 1);
    iconSizeGroupBox->setLayout(layout);
}

void MainWindow::createActions()
{
    addImageAct = new QAction(tr("&Add Image..."), this);
    addImageAct->setShortcut(tr("Ctrl+A"));
    connect(addImageAct, SIGNAL(triggered()), this, SLOT(addImage()));

    removeAllImagesAct = new QAction(tr("&Remove All Images"), this);
    removeAllImagesAct->setShortcut(tr("Ctrl+R"));
    connect(removeAllImagesAct, SIGNAL(triggered()),
            this, SLOT(removeAllImages()));

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    styleActionGroup = new QActionGroup(this);
    foreach (QString styleName, QStyleFactory::keys()) {
        QAction *action = new QAction(styleActionGroup);
        action->setText(tr("%1 Style").arg(styleName));
        action->setData(styleName);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(changeStyle(bool)));
    }

    guessModeStateAct = new QAction(tr("&Guess Image Mode/State"), this);
    guessModeStateAct->setCheckable(true);
    guessModeStateAct->setChecked(true);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(addImageAct);
    fileMenu->addAction(removeAllImagesAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    foreach (QAction *action, styleActionGroup->actions())
        viewMenu->addAction(action);
    viewMenu->addSeparator();
    viewMenu->addAction(guessModeStateAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createContextMenu()
{
    imagesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    imagesTable->addAction(addImageAct);
    imagesTable->addAction(removeAllImagesAct);
}

void MainWindow::checkCurrentStyle()
{
    foreach (QAction *action, styleActionGroup->actions()) {
        QString styleName = action->data().toString();
        QStyle *candidate = QStyleFactory::create(styleName);
        Q_ASSERT(candidate);
        if (candidate->metaObject()->className()
                == QApplication::style()->metaObject()->className()) {
            action->trigger();
            return;
        }
        delete candidate;
    }
}
