#include <QtGui>

#include "window.h"

Window::Window()
{
    operations << NoTransformation << NoTransformation << NoTransformation;

    QStringList operationStrings;
    operationStrings << tr("No transformation")
                     << tr("Rotate clockwise by 60\xB0")
                     << tr("Scale to 75%")
                     << tr("Translate by (50, 50)");

    shapeComboBox = new QComboBox(this);
    shapeComboBox->insertItem(tr("Clock"));
    shapeComboBox->insertItem(tr("House"));
    shapeComboBox->insertItem(tr("Truck"));

    firstOperation = new QComboBox(this);
    firstOperation->insertStringList(operationStrings);
    secondOperation = new QComboBox(this);
    secondOperation->insertStringList(operationStrings);
    thirdOperation = new QComboBox(this);
    thirdOperation->insertStringList(operationStrings);

    operationsList << NoTransformation << Rotate << Scale << Translate;

    originalRenderArea = new RenderArea(this);
    firstRenderArea = new RenderArea(this);
    secondRenderArea = new RenderArea(this);
    thirdRenderArea = new RenderArea(this);

    setupShapes();

    connect(firstOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));
    connect(secondOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));
    connect(thirdOperation, SIGNAL(activated(int)),
            this, SLOT(changeOperations(int)));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(originalRenderArea, 0, 0);
    layout->addWidget(firstRenderArea, 0, 1);
    layout->addWidget(secondRenderArea, 0, 2);
    layout->addWidget(thirdRenderArea, 0, 3);
    layout->addWidget(shapeComboBox, 1, 0);
    layout->addWidget(firstOperation, 1, 1);
    layout->addWidget(secondOperation, 1, 2);
    layout->addWidget(thirdOperation, 1, 3);

    shapeSelected(0);

    setWindowTitle(tr("Transformations"));
}

void Window::setupShapes()
{
    QPainterPath truck;
    truck.setFillRule(Qt::WindingFill);
    truck.moveTo(0.0, 87.0);
    truck.lineTo(0.0, 60.0);
    truck.lineTo(10.0, 60.0);
    truck.lineTo(35.0, 35.0);
    truck.lineTo(100.0, 35.0);
    truck.lineTo(100.0, 87.0);
    truck.lineTo(0.0, 87.0);
    truck.moveTo(17.0, 60.0);
    truck.lineTo(55.0, 60.0);
    truck.lineTo(55.0, 40.0);
    truck.lineTo(37.0, 40.0);
    truck.lineTo(17.0, 60.0);
    truck.addEllipse(17.0, 75.0, 25.0, 25.0);
    truck.addEllipse(63.0, 75.0, 25.0, 25.0);

    QPainterPath clock;
    clock.addEllipse(-50.0, -50.0, 100.0, 100.0);
    clock.addEllipse(-48.0, -48.0, 96.0, 96.0);
    clock.moveTo(0.0, 0.0);
    clock.lineTo(-2.0, -2.0);
    clock.lineTo(0.0, -42.0);
    clock.lineTo(2.0, -2.0);
    clock.lineTo(0.0, 0.0);
    clock.moveTo(0.0, 0.0);
    clock.lineTo(2.732, -0.732);
    clock.lineTo(24.495, 14.142);
    clock.lineTo(0.732, 2.732);
    clock.lineTo(0.0, 0.0);

    QPainterPath house;
    house.moveTo(-45.0, -20.0);
    house.lineTo(0.0, -45.0);
    house.lineTo(45.0, -20.0);
    house.lineTo(45.0, 45.0);
    house.lineTo(-45.0, 45.0);
    house.lineTo(-45.0, -20.0);
    house.addRect(15.0, 5.0, 20.0, 35.0);
    house.addRect(-35.0, -15.0, 25.0, 25.0);

    shapesList.append(clock);
    shapesList.append(house);
    shapesList.append(truck);

    connect(shapeComboBox, SIGNAL(activated(int)),
            this, SLOT(shapeSelected(int)));
}

void Window::changeOperations(int index)
{
    Operation operation = operationsList[index];

    if (sender() == firstOperation)
        operations[0] = operation;
    else if (sender() == secondOperation)
        operations[1] = operation;
    else if (sender() == thirdOperation)
        operations[2] = operation;

    QList<Operation> paintOperations;

    paintOperations << operations[0];
    firstRenderArea->setOperations(paintOperations);

    paintOperations << operations[1];
    secondRenderArea->setOperations(paintOperations);

    paintOperations << operations[2];
    thirdRenderArea->setOperations(paintOperations);
}

void Window::shapeSelected(int index)
{
    QPainterPath shape = shapesList[index];

    originalRenderArea->setShape(shape);
    firstRenderArea->setShape(shape);
    secondRenderArea->setShape(shape);
    thirdRenderArea->setShape(shape);
}
