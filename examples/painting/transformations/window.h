#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMap>
#include <QMainWindow>

#include "paintwidget.h"

class QComboBox;
class QFrame;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void changeOperations(int row);
    void setShape(QAction *action);

private:
    void setupFrame(QFrame *frame);
    void setupShapes();

    PaintWidget *originalPaintWidget;
    PaintWidget *firstPaintWidget;
    PaintWidget *secondPaintWidget;
    PaintWidget *thirdPaintWidget;
    QComboBox *firstOperation;
    QComboBox *secondOperation;
    QComboBox *thirdOperation;
    QList<Operation> operations;
    QList<Operation> operationsList;
    QMap<QAction*,QPainterPath> shapesMap;
};

#endif
