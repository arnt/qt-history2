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

private:
    void setupFrame(QFrame *frame);

    PaintWidget *firstPaintWidget;
    PaintWidget *secondPaintWidget;
    PaintWidget *thirdPaintWidget;
    QComboBox *firstOperation;
    QComboBox *secondOperation;
    QComboBox *thirdOperation;
    QList<Operation> operations;
    QMap<int,Operation> operationMap;
};

#endif
