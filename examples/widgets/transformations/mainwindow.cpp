#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    operations << NoTransformation << NoTransformation << NoTransformation;

    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"), this, SLOT(close()));
    quitAction->setShortcut(tr("Ctrl+Q"));

    menuBar()->addMenu(fileMenu);

    QFrame *frame = new QFrame(this);
    setupFrame(frame);

    setCentralWidget(frame);
    setWindowTitle(tr("Painting Demonstration"));
}

void MainWindow::setupFrame(QFrame *frame)
{
    QStringList operationStrings;
    operationStrings << tr("No transformation") << tr("Rotate") << tr("Scale")
                     << tr("Translate");

    QLabel *originalLabel = new QLabel(tr("Original shape"));
    firstOperation = new QComboBox(frame);
    firstOperation->insertStringList(operationStrings);
    secondOperation = new QComboBox(frame);
    secondOperation->insertStringList(operationStrings);
    thirdOperation = new QComboBox(frame);
    thirdOperation->insertStringList(operationStrings);

    operationMap[0] = NoTransformation;
    operationMap[1] = Rotate;
    operationMap[2] = Scale;
    operationMap[3] = Translate;

    PaintWidget *originalPaintWidget = new PaintWidget(frame, true);
    firstPaintWidget = new PaintWidget(frame);
    secondPaintWidget = new PaintWidget(frame);
    thirdPaintWidget = new PaintWidget(frame);

    connect(firstOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));
    connect(secondOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));
    connect(thirdOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));

    QGridLayout *layout = new QGridLayout(frame);
    layout->addWidget(originalPaintWidget, 0, 0);
    layout->addWidget(firstPaintWidget, 0, 1);
    layout->addWidget(secondPaintWidget, 0, 2);
    layout->addWidget(thirdPaintWidget, 0, 3);
    layout->addWidget(originalLabel, 1, 0);
    layout->addWidget(firstOperation, 1, 1);
    layout->addWidget(secondOperation, 1, 2);
    layout->addWidget(thirdOperation, 1, 3);
}

void MainWindow::changeOperations(int row)
{
    int index;

    if (sender() == firstOperation)
        index = 0;
    else if (sender() == secondOperation)
        index = 1;
    else if (sender() == thirdOperation)
        index = 2;
    
    Operation operation = operationMap[row];
    operations[index] = operation;

    QList<Operation> paintOperations;

    paintOperations << operations[0];
    firstPaintWidget->setOperations(paintOperations);

    paintOperations << operations[1];
    secondPaintWidget->setOperations(paintOperations);

    paintOperations << operations[2];
    thirdPaintWidget->setOperations(paintOperations);
}
