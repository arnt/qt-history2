#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QComboBox;
class QGroupBox;
class QSpinBox;
class QStackedWidget;
class SlidersGroup;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    void createControls(const QString &title);

    QGroupBox *controlsGroup;
    SlidersGroup *horizontalSliders;
    SlidersGroup *verticalSliders;
    QStackedWidget *pages;

    QComboBox *orientationCombo;
    QSpinBox *minimumSpinBox;
    QSpinBox *maximumSpinBox;
    QSpinBox *valueSpinBox;
};

#endif
