#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QPainterPath>
#include <QWidget>

#include "renderarea.h"

class QComboBox;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void changeOperations(int index);
    void shapeSelected(int index);

private:
    void setupShapes();

    RenderArea *originalRenderArea;
    RenderArea *firstRenderArea;
    RenderArea *secondRenderArea;
    RenderArea *thirdRenderArea;
    QComboBox *shapeComboBox;
    QComboBox *firstOperation;
    QComboBox *secondOperation;
    QComboBox *thirdOperation;
    QList<Operation> operations;
    QList<Operation> operationsList;
    QList<QPainterPath> shapesList;
};

#endif
