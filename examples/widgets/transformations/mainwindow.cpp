#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    operations << NoTransformation << NoTransformation << NoTransformation;

    QFrame *frame = new QFrame(this);
    setupFrame(frame);

    QMenu *fileMenu = new QMenu(tr("&File"));
    QAction *quitAction = fileMenu->addAction(tr("E&xit"), this, SLOT(close()));
    quitAction->setShortcut(tr("Ctrl+Q"));
    menuBar()->addMenu(fileMenu);

    setupShapes();

    setCentralWidget(frame);
    setWindowTitle(tr("Transformations"));
}

void MainWindow::setupFrame(QFrame *frame)
{
    QStringList operationStrings;
    operationStrings << tr("No transformation")
                     << tr("Rotate clockwise by 60 degrees")
                     << tr("Scale to 75%")
                     << tr("Translate by (50, 50)");

    QLabel *originalLabel = new QLabel(tr("Original shape"));
    originalLabel->setAlignment(Qt::AlignCenter);
    firstOperation = new QComboBox(frame);
    firstOperation->insertStringList(operationStrings);
    secondOperation = new QComboBox(frame);
    secondOperation->insertStringList(operationStrings);
    thirdOperation = new QComboBox(frame);
    thirdOperation->insertStringList(operationStrings);

    operationsList << NoTransformation << Rotate << Scale << Translate;

    originalPaintWidget = new PaintWidget(frame);
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

void MainWindow::setupShapes()
{
    QMenu *shapesMenu = new QMenu(tr("&Shapes"));
    QActionGroup *shapeActions = new QActionGroup(this);
    QAction *clockAction = shapesMenu->addAction(tr("&Clock"));
    shapeActions->addAction(clockAction);
    QAction *houseAction = shapesMenu->addAction(tr("&House"));
    shapeActions->addAction(houseAction);
    QAction *truckAction = shapesMenu->addAction(tr("&Truck"));
    shapeActions->addAction(truckAction);
    menuBar()->addMenu(shapesMenu);

    QPainterPath truck;
    truck.setFillRule(Qt::WindingFill);
    truck.moveTo(0, 87);
    truck.lineTo(0, 60);
    truck.lineTo(10, 60);
    truck.lineTo(35, 35);
    truck.lineTo(100, 35);
    truck.lineTo(100, 87);
    truck.lineTo(0, 87);
    truck.moveTo(17, 60);
    truck.lineTo(55, 60);
    truck.lineTo(55, 40);
    truck.lineTo(37, 40);
    truck.lineTo(17, 60);
    truck.addEllipse(20, 75, 25, 25);
    truck.addEllipse(60, 75, 25, 25);

    QPainterPath clock;
    clock.addEllipse(-50, -50, 100, 100);
    clock.addEllipse(-48, -48, 96, 96);
    clock.moveTo(0, 0);
    clock.lineTo(-2, -2);
    clock.lineTo(0, -42);
    clock.lineTo(2, -2);
    clock.lineTo(0, 0);
    clock.moveTo(0, 0);
    clock.lineTo(2.732, -0.732);
    clock.lineTo(24.495, 14.142);
    clock.lineTo(0.732, 2.732);
    clock.lineTo(0, 0);

    QPainterPath house;
    house.moveTo(-45, -20);
    house.lineTo(  0, -45);
    house.lineTo( 45, -20);
    house.lineTo( 45,  45);
    house.lineTo(-45,  45);
    house.lineTo(-45, -20);
    house.addRect(15, 5, 20, 35);
    house.addRect(-35, -15, 25, 25);

    shapesMap[truckAction] = truck;
    shapesMap[clockAction] = clock;
    shapesMap[houseAction] = house;

    connect(shapesMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(setShape(QAction *)));

    setShape(houseAction);
}

void MainWindow::changeOperations(int row)
{
    Operation operation = operationsList[row];

    if (sender() == firstOperation)
        operations[0] = operation;
    else if (sender() == secondOperation)
        operations[1] = operation;
    else if (sender() == thirdOperation)
        operations[2] = operation;

    QList<Operation> paintOperations;

    paintOperations << operations[0];
    firstPaintWidget->setOperations(paintOperations);

    paintOperations << operations[1];
    secondPaintWidget->setOperations(paintOperations);

    paintOperations << operations[2];
    thirdPaintWidget->setOperations(paintOperations);
}

void MainWindow::setShape(QAction *action)
{
    QPainterPath shape = shapesMap[action];
    action->setChecked(true);
    originalPaintWidget->setShape(shape);
    firstPaintWidget->setShape(shape);
    secondPaintWidget->setShape(shape);
    thirdPaintWidget->setShape(shape);
}
