#include <QtGui>

#include "window.h"

Window::Window()
{
    horizontalSliders = new SlidersGroup(Qt::Horizontal,
        tr("Horizontal sliders"), 0);
    verticalSliders = new SlidersGroup(Qt::Vertical,
        tr("Vertical sliders"), 0);

    pages = new QStackedWidget(this);
    pages->addWidget(horizontalSliders);
    pages->addWidget(verticalSliders);

    createControls(tr("Controls"));

    connect(horizontalSliders, SIGNAL(valueChanged(int)),
            verticalSliders, SLOT(setValue(int)));
    connect(verticalSliders, SIGNAL(valueChanged(int)),
            this, SLOT(setValue(int)));
    connect(this, SIGNAL(valueChanged(int)),
            horizontalSliders, SLOT(setValue(int)));

    connect(this, SIGNAL(minimumChanged(int)),
            horizontalSliders, SLOT(setMinimum(int)));
    connect(this, SIGNAL(maximumChanged(int)),
            horizontalSliders, SLOT(setMaximum(int)));
    connect(this, SIGNAL(appearanceInverted(bool)),
            horizontalSliders, SLOT(invertAppearance(bool)));
    connect(this, SIGNAL(controlsInverted(bool)),
            horizontalSliders, SLOT(invertControls(bool)));
    connect(this, SIGNAL(minimumChanged(int)),
            verticalSliders, SLOT(setMinimum(int)));
    connect(this, SIGNAL(maximumChanged(int)),
            verticalSliders, SLOT(setMaximum(int)));
    connect(this, SIGNAL(appearanceInverted(bool)),
            verticalSliders, SLOT(invertAppearance(bool)));
    connect(this, SIGNAL(controlsInverted(bool)),
            verticalSliders, SLOT(invertControls(bool)));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controlsGroup);
    layout->addWidget(pages);

    setWindowTitle(tr("Sliders"));
}

void Window::createControls(const QString &title)
{
    controlsGroup = new QGroupBox(title, this);

    QLabel *minimumLabel = new QLabel(tr("Minimum value"), controlsGroup);
    QLabel *maximumLabel = new QLabel(tr("Maximum value"), controlsGroup);
    QLabel *valueLabel = new QLabel(tr("Current value"), controlsGroup);

    QCheckBox *invertedAppearance = new QCheckBox(tr("Inverted appearance"),
                                                  controlsGroup);
    QCheckBox *invertedControls = new QCheckBox(tr("Inverted controls"),
                                                controlsGroup);

    minimumSpinBox = new QSpinBox(controlsGroup);
    minimumSpinBox->setRange(-20, 20);
    minimumSpinBox->setSingleStep(1);
    minimumSpinBox->setValue(-20);

    maximumSpinBox = new QSpinBox(controlsGroup);
    maximumSpinBox->setRange(-20, 20);
    maximumSpinBox->setSingleStep(1);
    maximumSpinBox->setValue(20);

    valueSpinBox = new QSpinBox(controlsGroup);
    valueSpinBox->setRange(-20, 20);
    valueSpinBox->setSingleStep(1);
    valueSpinBox->setValue(10);

    orientationCombo = new QComboBox(controlsGroup);
    orientationCombo->addItem(tr("Horizontal sliders"));
    orientationCombo->addItem(tr("Vertical sliders"));

    connect(orientationCombo, SIGNAL(activated(int)),
            pages, SLOT(setCurrentIndex(int)));
    connect(valueSpinBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(valueChanged(int)));
    connect(minimumSpinBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(minimumChanged(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(maximumChanged(int)));
    connect(invertedAppearance, SIGNAL(toggled(bool)),
            this, SIGNAL(appearanceInverted(bool)));
    connect(invertedControls, SIGNAL(toggled(bool)),
            this, SIGNAL(controlsInverted(bool)));

    QGridLayout *controlsLayout = new QGridLayout(controlsGroup);
    controlsLayout->addWidget(minimumLabel, 0, 0);
    controlsLayout->addWidget(maximumLabel, 1, 0);
    controlsLayout->addWidget(valueLabel, 2, 0);
    controlsLayout->addWidget(minimumSpinBox, 0, 1);
    controlsLayout->addWidget(maximumSpinBox, 1, 1);
    controlsLayout->addWidget(valueSpinBox, 2, 1);
    controlsLayout->addWidget(invertedAppearance, 0, 2);
    controlsLayout->addWidget(invertedControls, 1, 2);
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
    slider->setFocusPolicy(Qt::ClickFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setRange(-20, 20);
    slider->setSingleStep(1);
    slider->setValue(0);

    scrollBar = new QScrollBar(orientation, this);
    scrollBar->setFocusPolicy(Qt::ClickFocus);
    scrollBar->setRange(-20, 20);
    scrollBar->setValue(0);

    dial = new QDial(this);
    dial->setRange(-20, 20);
    dial->setValue(0);

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
