#include <QtGui>

#include "window.h"

Window::Window()
{
    horizontalSliders = new SlidersGroup(Qt::Horizontal, tr("Horizontal"));
    verticalSliders = new SlidersGroup(Qt::Vertical, tr("Vertical"));

    pages = new QStackedWidget(this);
    pages->addWidget(horizontalSliders);
    pages->addWidget(verticalSliders);

    createControls(tr("Controls"));

    connect(horizontalSliders, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setValue(int)));
    connect(verticalSliders, SIGNAL(valueChanged(int)),
            valueSpinBox, SLOT(setValue(int)));
    connect(valueSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setValue(int)));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controlsGroup);
    layout->addWidget(pages);

    minimumSpinBox->setValue(0);
    maximumSpinBox->setValue(20);
    valueSpinBox->setValue(5);

    setWindowTitle(tr("Sliders"));
}

void Window::createControls(const QString &title)
{
    controlsGroup = new QGroupBox(title, this);

    QLabel *minimumLabel = new QLabel(tr("Minimum value:"), controlsGroup);
    QLabel *maximumLabel = new QLabel(tr("Maximum value:"), controlsGroup);
    QLabel *valueLabel = new QLabel(tr("Current value:"), controlsGroup);

    QCheckBox *invertedAppearance = new QCheckBox(tr("Inverted appearance"),
                                                  controlsGroup);
    QCheckBox *invertedKeyBindings = new QCheckBox(tr("Inverted key bindings"),
                                                   controlsGroup);

    minimumSpinBox = new QSpinBox(controlsGroup);
    minimumSpinBox->setRange(-100, 100);
    minimumSpinBox->setSingleStep(1);

    maximumSpinBox = new QSpinBox(controlsGroup);
    maximumSpinBox->setRange(-100, 100);
    maximumSpinBox->setSingleStep(1);

    valueSpinBox = new QSpinBox(controlsGroup);
    valueSpinBox->setRange(-100, 100);
    valueSpinBox->setSingleStep(1);

    orientationCombo = new QComboBox(controlsGroup);
    orientationCombo->addItem(tr("Horizontal slider-like widgets"));
    orientationCombo->addItem(tr("Vertical slider-like widgets"));

    connect(orientationCombo, SIGNAL(activated(int)),
            pages, SLOT(setCurrentIndex(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setMinimum(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setMinimum(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setMaximum(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setMaximum(int)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            horizontalSliders, SLOT(invertAppearance(bool)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            verticalSliders, SLOT(invertAppearance(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            horizontalSliders, SLOT(invertControls(bool)));
    connect(invertedKeyBindings, SIGNAL(toggled(bool)),
            verticalSliders, SLOT(invertControls(bool)));

    QGridLayout *controlsLayout = new QGridLayout(controlsGroup);
    controlsLayout->addWidget(minimumLabel, 0, 0);
    controlsLayout->addWidget(maximumLabel, 1, 0);
    controlsLayout->addWidget(valueLabel, 2, 0);
    controlsLayout->addWidget(minimumSpinBox, 0, 1);
    controlsLayout->addWidget(maximumSpinBox, 1, 1);
    controlsLayout->addWidget(valueSpinBox, 2, 1);
    controlsLayout->addWidget(invertedAppearance, 0, 2);
    controlsLayout->addWidget(invertedKeyBindings, 1, 2);
    controlsLayout->addWidget(orientationCombo, 3, 0, 1, 3);
}

void Window::setValue(int value)
{
    valueSpinBox->setValue(value);
}

SlidersGroup::SlidersGroup(Qt::Orientation orientation, const QString &title,
                           QWidget *parent)
    : QGroupBox(title, parent)
{
    slider = new QSlider(orientation, this);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);

    scrollBar = new QScrollBar(orientation, this);
    scrollBar->setFocusPolicy(Qt::StrongFocus);

    dial = new QDial(this);
    dial->setFocusPolicy(Qt::StrongFocus);

    connect(slider, SIGNAL(valueChanged(int)), scrollBar, SLOT(setValue(int)));
    connect(scrollBar, SIGNAL(valueChanged(int)), dial, SLOT(setValue(int)));
    connect(dial, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    QBoxLayout::Direction direction;

    if (orientation == Qt::Horizontal)
        direction = QBoxLayout::TopToBottom;
    else
        direction = QBoxLayout::LeftToRight;

    QBoxLayout *slidersLayout = new QBoxLayout(direction, this);
    slidersLayout->addWidget(slider);
    slidersLayout->addWidget(scrollBar);
    slidersLayout->addWidget(dial);
}

void SlidersGroup::setValue(int value)
{
    slider->setValue(value);
}

void SlidersGroup::setMinimum(int value)
{
    slider->setMinimum(value);
    scrollBar->setMinimum(value);
    dial->setMinimum(value);
}

void SlidersGroup::setMaximum(int value)
{
    slider->setMaximum(value);
    scrollBar->setMaximum(value);
    dial->setMaximum(value);
}

void SlidersGroup::invertAppearance(bool invert)
{
    slider->setInvertedAppearance(invert);
    scrollBar->setInvertedAppearance(invert);
    dial->setInvertedAppearance(invert);
}

void SlidersGroup::invertControls(bool invert)
{
    slider->setInvertedControls(invert);
    scrollBar->setInvertedControls(invert);
    dial->setInvertedControls(invert);
}
