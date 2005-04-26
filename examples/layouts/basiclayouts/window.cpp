#include <QtGui>

#include "window.h"

Window::Window()
{
    QMenuBar *menuBar = new QMenuBar;

    QMenu* fileMenu = new QMenu(tr("&File"));
    QAction *quitAction = fileMenu->addAction(tr("&Quit"));
    menuBar->addMenu(fileMenu);

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QGroupBox *horizontalGroup = new QGroupBox(tr("Horizontal layout"));
    QBoxLayout *horizontalLayout = new QHBoxLayout;
    for (int i = 1; i <= 4; ++i) {
	QPushButton *button = new QPushButton;
	button->setText(tr("Button %1").arg(i));
	horizontalLayout->addWidget(button, 10);
    }
    horizontalGroup->setLayout(horizontalLayout);

    QGroupBox *gridGroup = new QGroupBox(tr("Grid layout"));
    QTextEdit *gridEditor = new QTextEdit;
    gridEditor->setPlainText(tr("This widget will take up three rows in "
                                "the grid layout."));

    QGridLayout *gridLayout = new QGridLayout;
    for (int row = 0; row < 3; ++row) {
	QLabel *label = new QLabel(tr("Line %1").arg(row+1));
	QLineEdit *lineEdit = new QLineEdit;
	gridLayout->addWidget(label, row, 0);
	gridLayout->addWidget(lineEdit, row, 1);
    }
    gridLayout->addWidget(gridEditor, 0, 2, 3, 1);
    gridLayout->setColumnStretch(1, 10);
    gridLayout->setColumnStretch(2, 20);
    gridGroup->setLayout(gridLayout);

    QTextEdit *bigEditor = new QTextEdit;
    bigEditor->setPlainText(tr("This widget will take up all the remaining "
                               "space in the top-level layout."));

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(cancelButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QHBoxLayout *paddedLayout = new QHBoxLayout;
    paddedLayout->addStretch(1);
    paddedLayout->addWidget(okButton);
    paddedLayout->addWidget(cancelButton);

    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(horizontalGroup);
    mainLayout->addWidget(gridGroup);
    mainLayout->addWidget(bigEditor);
    mainLayout->addLayout(paddedLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Basic Layouts"));
}
