#include <QtGui>

#include "characterwidget.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    QWidget *centralWidget = new QWidget(this);

    QLabel *fontLabel = new QLabel(tr("Font:"), centralWidget);
    fontCombo = new QComboBox(centralWidget);
    QLabel *styleLabel = new QLabel(tr("Style:"), centralWidget);
    styleCombo = new QComboBox(centralWidget);

    view = new QWidgetView(centralWidget);
    characterWidget = new CharacterWidget;
    view->setWidget(characterWidget);

    findFonts();
    findStyles();

    lineEdit = new QLineEdit(centralWidget);
    QPushButton *clipboardButton = new QPushButton(tr("&To clipboard"), centralWidget);

    clipboard = QApplication::clipboard();

    connect(fontCombo, SIGNAL(activated(const QString &)),
            this, SLOT(findStyles()));
    connect(fontCombo, SIGNAL(activated(const QString &)),
            characterWidget, SLOT(updateFont(const QString &)));
    connect(styleCombo, SIGNAL(activated(const QString &)),
            characterWidget, SLOT(updateStyle(const QString &)));
    connect(characterWidget, SIGNAL(characterSelected(const QString &)),
            this, SLOT(insertCharacter(const QString &)));
    connect(clipboardButton, SIGNAL(clicked()), this, SLOT(updateClipboard()));

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(fontLabel);
    controlsLayout->addWidget(fontCombo, 1);
    controlsLayout->addWidget(styleLabel);
    controlsLayout->addWidget(styleCombo, 1);
    controlsLayout->addStretch(1);

    QHBoxLayout *lineLayout = new QHBoxLayout;
    lineLayout->addWidget(lineEdit, 1);
    lineLayout->addSpacing(12);
    lineLayout->addWidget(clipboardButton);

    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addLayout(controlsLayout);
    centralLayout->addWidget(view, 1);
    centralLayout->addSpacing(4);
    centralLayout->addLayout(lineLayout);

    setCentralWidget(centralWidget);
    setWindowTitle(tr("Character Map"));
}

void MainWindow::findFonts()
{
    QFontDatabase fontDatabase;
    fontCombo->clear();

    QString family;
    foreach (family, fontDatabase.families())
        fontCombo->insertItem(family);
}

void MainWindow::findStyles()
{
    QFontDatabase fontDatabase;
    QString currentItem = styleCombo->currentText();
    styleCombo->clear();

    QString style;
    foreach (style, fontDatabase.styles(fontCombo->currentText()))
        styleCombo->insertItem(style);

    if (!styleCombo->contains(currentItem))
        styleCombo->setCurrentItem(0);
}

void MainWindow::insertCharacter(const QString &character)
{
    lineEdit->insert(character);
}

void MainWindow::updateClipboard()
{
    clipboard->setText(lineEdit->text());
}
