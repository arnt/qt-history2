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
                 QWidget *parent);

signals:
    void valueChanged(int);

public slots:
    void setValue(int);
    void setMinimum(int);
    void setMaximum(int);
    void invertAppearance(bool);
    void invertControls(bool);

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

signals:
    void valueChanged(int);
    void minimumChanged(int);
    void maximumChanged(int);
    void appearanceInverted(bool);
    void controlsInverted(bool);

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
