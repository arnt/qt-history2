#include <QtGui>

#include "norwegianwoodstyle.h"
#include "widgetgallery.h"

WidgetGallery::WidgetGallery(QWidget *parent)
    : QDialog(parent)
{
    styleComboBox = new QComboBox(this);
    styleComboBox->addItem("NorwegianWood");
    styleComboBox->addItems(QStyleFactory::keys());

    styleLabel = new QLabel(tr("&Style:"), this);
    styleLabel->setBuddy(styleComboBox);

    disableWidgetsCheckBox = new QCheckBox(tr("&Disable widgets"), this);

    createTopLeftGroupBox();
    createTopRightGroupBox();
    createBottomLeftTabWidget();
    createBottomRightGroupBox();
    createProgressBar();

    connect(styleComboBox, SIGNAL(activated(const QString &)),
            this, SLOT(changeStyle(const QString &)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            topLeftGroupBox, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            topRightGroupBox, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            bottomLeftTabWidget, SLOT(setDisabled(bool)));
    connect(disableWidgetsCheckBox, SIGNAL(toggled(bool)),
            bottomRightGroupBox, SLOT(setDisabled(bool)));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(styleLabel);
    topLayout->addWidget(styleComboBox);
    topLayout->addStretch(1);
    topLayout->addWidget(disableWidgetsCheckBox);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addLayout(topLayout, 0, 0, 1, 2);
    mainLayout->addWidget(topLeftGroupBox, 1, 0);
    mainLayout->addWidget(topRightGroupBox, 1, 1);
    mainLayout->addWidget(bottomLeftTabWidget, 2, 0);
    mainLayout->addWidget(bottomRightGroupBox, 2, 1);
    mainLayout->addWidget(progressBar, 3, 0, 1, 2);
    mainLayout->setRowStretch(1, 1);
    mainLayout->setRowStretch(2, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);

    setWindowTitle(tr("Styles"));
    changeStyle("NorwegianWood");
}

void WidgetGallery::changeStyle(const QString &styleName)
{
    if (styleName == "NorwegianWood") {
        QApplication::setStyle(new NorwegianWoodStyle);
    } else {
        QApplication::setStyle(QStyleFactory::create(styleName));
    }
}

void WidgetGallery::advanceProgressBar()
{
    int curVal = progressBar->value();
    int maxVal = progressBar->maximum();
    progressBar->setValue(curVal + (maxVal - curVal) / 100);
}

void WidgetGallery::createTopLeftGroupBox()
{
    topLeftGroupBox = new QGroupBox(tr("Group 1"), this);

    radioButton1 = new QRadioButton(tr("Radio button 1"), topLeftGroupBox);
    radioButton2 = new QRadioButton(tr("Radio button 2"), topLeftGroupBox);
    radioButton3 = new QRadioButton(tr("Radio button 3"), topLeftGroupBox);
    radioButton1->setChecked(true);

    checkBox = new QCheckBox(tr("Tri-state check box"), topLeftGroupBox);
    checkBox->setTristate(true);
    checkBox->setCheckState(Qt::PartiallyChecked);

    QVBoxLayout *layout = new QVBoxLayout(topLeftGroupBox);
    layout->addWidget(radioButton1);
    layout->addWidget(radioButton2);
    layout->addWidget(radioButton3);
    layout->addWidget(checkBox);
    layout->addStretch(1);
}

void WidgetGallery::createTopRightGroupBox()
{
    topRightGroupBox = new QGroupBox(tr("Group 2"), this);

    normalPushButton = new QPushButton(tr("Normal Push Button"),
                                       topRightGroupBox);
    togglePushButton = new QPushButton(tr("Toggle Push Button"),
                                       topRightGroupBox);
    togglePushButton->setCheckable(true);
    togglePushButton->setChecked(true);
    flatPushButton = new QPushButton(tr("Flat Push Button"), topRightGroupBox);
    flatPushButton->setFlat(true);

    QVBoxLayout *layout = new QVBoxLayout(topRightGroupBox);
    layout->addWidget(normalPushButton);
    layout->addWidget(togglePushButton);
    layout->addWidget(flatPushButton);
    layout->addStretch(1);
}

void WidgetGallery::createBottomLeftTabWidget()
{
    bottomLeftTabWidget = new QTabWidget(this);
    bottomLeftTabWidget->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Ignored);

    QHBoxWidget *tab1 = new QHBoxWidget;
    tab1->setMargin(5);
    tableWidget = new QTableWidget(10, 10, tab1);

    QHBoxWidget *tab2 = new QHBoxWidget;
    tab2->setMargin(5);
    textEdit = new QTextEdit(tab2);
    textEdit->setPlainText(tr("Au clair de la lune,\n"
                              "Mon ami Pierrot,\n"
                              "Pr�te moi ta plume\n"
                              "Pour �crire un mot.\n"
                              "Ma chandelle est morte,\n"
                              "Je n'ai plus de feu.\n"
                              "Ouvre moi ta porte,\n"
                              "Pour l'amour de Dieu.\n"));

    bottomLeftTabWidget->addTab(tab1, tr("&Table"));
    bottomLeftTabWidget->addTab(tab2, tr("Text &Edit"));
}

void WidgetGallery::createBottomRightGroupBox()
{
    bottomRightGroupBox = new QGroupBox(tr("Group 3"), this);
    bottomRightGroupBox->setCheckable(true);
    bottomRightGroupBox->setChecked(true);

    lineEdit = new QLineEdit("s3cRe7", bottomRightGroupBox);
    lineEdit->setEchoMode(QLineEdit::Password);

    spinBox = new QSpinBox(bottomRightGroupBox);

    dateTimeEdit = new QDateTimeEdit(bottomRightGroupBox);

    slider = new QSlider(Qt::Horizontal, bottomRightGroupBox);

    scrollBar = new QScrollBar(Qt::Horizontal, bottomRightGroupBox);

    dial = new QDial(bottomRightGroupBox);

    QGridLayout *layout = new QGridLayout(bottomRightGroupBox);
    layout->addWidget(lineEdit, 0, 0, 1, 2);
    layout->addWidget(spinBox, 1, 0, 1, 2);
    layout->addWidget(dateTimeEdit, 2, 0, 1, 2);
    layout->addWidget(slider, 3, 0);
    layout->addWidget(scrollBar, 4, 0);
    layout->addWidget(dial, 3, 1, 2, 1);
    layout->setRowStretch(5, 1);
}

void WidgetGallery::createProgressBar()
{
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 10000);
    progressBar->setValue(0);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advanceProgressBar()));
    timer->start(1000);
}
