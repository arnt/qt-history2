#include <QtGui>

#include "slidersgroup.h"
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
