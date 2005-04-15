#include <QtGui>

#include "iconpreviewarea.h"
#include "iconsizespinbox.h"
#include "window.h"

Window::Window()
{
    centralWidget = new QWidget;
    setCentralWidget(centralWidget);

    createPreviewGroupBox();
    createImagesGroupBox();
    createIconSizeGroupBox();
    createMenus();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(previewGroupBox, 0, 0, 2, 1);
    mainLayout->addWidget(imagesGroupBox, 0, 1);
    mainLayout->addWidget(iconSizeGroupBox, 1, 1);
    centralWidget->setLayout(mainLayout);

    setWindowTitle(tr("Icons"));
    checkCurrentStyle();
    largeRadioButton->toggle();
}

void Window::about()
{
    QMessageBox::about(this, tr("About Icons"),
            tr("The <b>Icons</b> example illustrates how Qt renders an icon in "
               "different modes (active, normal, and disabled) and states (on "
               "and off)."));
}

void Window::changeStyle(bool checked)
{
    if (!checked)
        return;

    QAction *action = qobject_cast<QAction *>(sender());
    QStyle *style = QStyleFactory::create(action->text());
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

void Window::changeSize()
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

void Window::changeIcon()
{
    QIcon icon;

    for (int row = 0; row < imagesTable->rowCount(); ++row) {
        QTableWidgetItem *item0 = imagesTable->item(row, 0);
        QTableWidgetItem *item1 = imagesTable->item(row, 1);
        QTableWidgetItem *item2 = imagesTable->item(row, 2);

        icon.addPixmap(qvariant_cast<QPixmap>(item0->data(PixmapRole)),
                       (item1->checkState() == Qt::Checked) ? QIcon::Disabled : QIcon::Normal,
                       (item2->checkState() == Qt::Checked) ? QIcon::On : QIcon::Off);
    }

    previewArea->setIcon(icon);
}

void Window::addImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"));
    if (!fileName.isEmpty()) {
        QImage image;
        if (image.load(fileName)) {
            int row = imagesTable->rowCount();
            imagesTable->setRowCount(row + 1);


            QString strippedFileName = QFileInfo(fileName).fileName();
            QTableWidgetItem *item0 = new QTableWidgetItem(strippedFileName);
            item0->setData(PixmapRole, QPixmap::fromImage(image));
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);

            QTableWidgetItem *item1 = new QTableWidgetItem(tr("Disabled"));
            item1->setCheckState(Qt::Unchecked);
            item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);

            QTableWidgetItem *item2 = new QTableWidgetItem(tr("On"));
            item2->setCheckState(Qt::Unchecked);
            item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);

            imagesTable->setItem(row, 0, item0);
            imagesTable->setItem(row, 1, item1);
            imagesTable->setItem(row, 2, item2);

            changeIcon();
        }
    }
}

void Window::resetImages()
{
    imagesTable->setRowCount(0);
    changeIcon();
}

void Window::createPreviewGroupBox()
{
    previewGroupBox = new QGroupBox(tr("Preview"));

    previewArea = new IconPreviewArea;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(previewArea);
    previewGroupBox->setLayout(layout);
}

void Window::createImagesGroupBox()
{
    imagesGroupBox = new QGroupBox(tr("Images"));
    imagesGroupBox->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);

    QStringList labels;
    labels << tr("Image") << tr("Mode") << tr("State");

    imagesTable = new QTableWidget;
    imagesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    imagesTable->setSelectionMode(QAbstractItemView::NoSelection);
    imagesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    imagesTable->setColumnCount(3);
    imagesTable->setHorizontalHeaderLabels(labels);

    addButton = new QPushButton(tr("&Add..."));
    resetButton = new QPushButton(tr("&Reset"));

    connect(imagesTable, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(changeIcon()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addImage()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetImages()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(imagesTable, 0, 0, 3, 1);
    layout->addWidget(addButton, 0, 1);
    layout->addWidget(resetButton, 1, 1);
    imagesGroupBox->setLayout(layout);
}

void Window::createIconSizeGroupBox()
{
    iconSizeGroupBox = new QGroupBox(tr("&Icon Size"));

    smallRadioButton = new QRadioButton;
    largeRadioButton = new QRadioButton;
    toolBarRadioButton = new QRadioButton;
    listViewRadioButton = new QRadioButton;
    iconViewRadioButton = new QRadioButton;
    otherRadioButton = new QRadioButton(tr("Other:"));

    otherSpinBox = new IconSizeSpinBox;
    otherSpinBox->setRange(8, 128);
    otherSpinBox->setValue(24);

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

void Window::createMenus()
{
    styleActionGroup = new QActionGroup(this);
    foreach (QString styleName, QStyleFactory::keys()) {
        QAction *action = new QAction(styleName, styleActionGroup);
        action->setCheckable(true);
        connect(action, SIGNAL(checked(bool)), this, SLOT(changeStyle(bool)));
    }

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    styleMenu = menuBar()->addMenu(tr("&Style"));
    foreach (QAction *action, styleActionGroup->actions())
        styleMenu->addAction(action);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void Window::checkCurrentStyle()
{
    foreach (QAction *action, styleActionGroup->actions()) {
        QString styleName = action->text();
        QStyle *candidate = QStyleFactory::create(styleName);
        if (candidate->metaObject()->className()
                == QApplication::style()->metaObject()->className()) {
            action->setChecked(true);
            return;
        }
        delete candidate;
    }
}
