#ifndef SLIDERS_H
#define SLIDERS_H

#include <QBoxLayout>
#include <QGroupBox>
#include <QWidget>

class QComboBox;
class QDial;
class QScrollBar;
class QSlider;
class QSpinBox;
class QStackedWidget;

class SlidersGroup : public QGroupBox
{
    Q_OBJECT

public:
    SlidersGroup(Qt::Orientation orientation, const QString &title,
                 QWidget *parent = 0);

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value);
    void setMinimum(int value);
    void setMaximum(int value);
    void invertAppearance(bool invert);
    void invertControls(bool invert);

private:
    QDial *dial;
    QScrollBar *scrollBar;
    QSlider *slider;
};

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void setValue(int);

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
