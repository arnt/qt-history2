#include <QtGui>

#include "window.h"

Window::Window()
{
    createSpinBoxes();
    createDateTimeEdits();
    createDoubleSpinBoxes();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(spinBoxesGroup);
    layout->addWidget(editsGroup);
    layout->addWidget(doubleSpinBoxesGroup);

    setWindowTitle(tr("Spinboxes"));
}

void Window::createSpinBoxes()
{
    spinBoxesGroup = new QGroupBox(tr("Spinboxes"), this);

    QLabel *integerLabel = new QLabel(tr("Enter a value between "
        "%1 and %2:").arg(-20).arg(20), spinBoxesGroup);
    QSpinBox *integerSpinBox = new QSpinBox(spinBoxesGroup);
    integerSpinBox->setRange(-20, 20);
    integerSpinBox->setSingleStep(1);
    integerSpinBox->setValue(0);

    QLabel *zoomLabel = new QLabel(tr("Enter a zoom value between "
        "%1 and %2:").arg(0).arg(1000), spinBoxesGroup);
    QSpinBox *zoomSpinBox = new QSpinBox(spinBoxesGroup);
    zoomSpinBox->setRange(0, 1000);
    zoomSpinBox->setSingleStep(10);
    zoomSpinBox->setSuffix(" %");
    zoomSpinBox->setSpecialValueText(tr("Automatic"));
    zoomSpinBox->setValue(100);

    QLabel *priceLabel = new QLabel(tr("Enter a price between "
        "%1 and %2:").arg(0).arg(999), spinBoxesGroup);
    QSpinBox *priceSpinBox = new QSpinBox(spinBoxesGroup);
    priceSpinBox->setRange(0, 999);
    priceSpinBox->setSingleStep(1);
    priceSpinBox->setPrefix("$");
    priceSpinBox->setValue(99);

    QVBoxLayout *spinBoxLayout = new QVBoxLayout(spinBoxesGroup);
    spinBoxLayout->addWidget(integerLabel);
    spinBoxLayout->addWidget(integerSpinBox);
    spinBoxLayout->addWidget(zoomLabel);
    spinBoxLayout->addWidget(zoomSpinBox);
    spinBoxLayout->addWidget(priceLabel);
    spinBoxLayout->addWidget(priceSpinBox);
}

void Window::createDateTimeEdits()
{
    editsGroup = new QGroupBox(tr("Date and time spin boxes"), this);

    QLabel *dateLabel = new QLabel(tr("Appointment date:"), editsGroup);
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate(),
                                                editsGroup);
    dateEdit->setDateRange(QDate(2005, 1, 1), QDate(2010, 12, 31));

    QLabel *timeLabel = new QLabel(tr("Appointment time:"), editsGroup);
    QDateTimeEdit *timeEdit = new QDateTimeEdit(QTime::currentTime(),
                                                editsGroup);
    timeEdit->setTimeRange(QTime(9, 0, 0, 0), QTime(16, 30, 0, 0));

    QLabel *meetingLabel = new QLabel(tr("Meeting date and time:"),
                                       editsGroup);
    meetingEdit = new QDateTimeEdit(QDateTime::currentDateTime(), editsGroup);
    meetingEdit->setDateRange(QDate(2004, 11, 1), QDate(2005, 11, 30));
    meetingEdit->setTimeRange(QTime(0, 0, 0, 0), QTime(23, 59, 59, 999));

    QLabel *formatLabel = new QLabel(tr("Format string for the meeting date "
        "and time"), editsGroup);

    QComboBox *formatComboBox = new QComboBox(editsGroup);
    formatComboBox->insertItem("yyyy-MM-dd hh:mm:ss (zzz ms)");
    formatComboBox->insertItem("hh:mm:ss MM/dd/yyyy");
    formatComboBox->insertItem("hh:mm:ss dd/MM/yyyy");
    formatComboBox->insertItem("dd MMM yy");
    formatComboBox->insertItem("ddd MMMM d yyyy");
    formatComboBox->insertItem("dddd MMMM d yyyy");
    formatComboBox->insertItem("hh:mm:ss");
    formatComboBox->insertItem("hh:mm ap");

    connect(formatComboBox, SIGNAL(activated(const QString &)),
            this, SLOT(setFormatString(const QString &)));

    QVBoxLayout *editsLayout = new QVBoxLayout(editsGroup);
    editsLayout->addWidget(dateLabel);
    editsLayout->addWidget(dateEdit);
    editsLayout->addWidget(timeLabel);
    editsLayout->addWidget(timeEdit);
    editsLayout->addWidget(meetingLabel);
    editsLayout->addWidget(meetingEdit);
    editsLayout->addWidget(formatLabel);
    editsLayout->addWidget(formatComboBox);
}

void Window::setFormatString(const QString &formatString)
{
    meetingEdit->setFormat(formatString);
}

void Window::createDoubleSpinBoxes()
{
    doubleSpinBoxesGroup = new QGroupBox(tr("Double precision spinboxes"), this);

    QLabel *precisionLabel = new QLabel(tr("Number of decimal places to show:"),
                                        doubleSpinBoxesGroup);
    QSpinBox *precisionSpinBox = new QSpinBox(doubleSpinBoxesGroup);
    precisionSpinBox->setRange(0, 14);
    precisionSpinBox->setValue(2);

    QLabel *doubleLabel = new QLabel(tr("Enter a value between "
        "%1 and %2:").arg(-20).arg(20), doubleSpinBoxesGroup);
    doubleSpinBox = new QDoubleSpinBox(doubleSpinBoxesGroup);
    doubleSpinBox->setRange(-20.0, 20.0);
    doubleSpinBox->setSingleStep(1.0);
    doubleSpinBox->setValue(0.0);

    QLabel *scaleLabel = new QLabel(tr("Enter a scale factor between "
        "%1 and %2:").arg(0).arg(1000.0), doubleSpinBoxesGroup);
    scaleSpinBox = new QDoubleSpinBox(doubleSpinBoxesGroup);
    scaleSpinBox->setRange(0.0, 1000.0);
    scaleSpinBox->setSingleStep(10.0);
    scaleSpinBox->setSuffix(" %");
    scaleSpinBox->setSpecialValueText(tr("No scaling"));
    scaleSpinBox->setValue(100.0);

    QLabel *priceLabel = new QLabel(tr("Enter a price between "
        "%1 and %2:").arg(0).arg(1000), doubleSpinBoxesGroup);
    priceSpinBox = new QDoubleSpinBox(doubleSpinBoxesGroup);
    priceSpinBox->setRange(0.0, 1000.0);
    priceSpinBox->setSingleStep(1.0);
    priceSpinBox->setPrefix("$");
    priceSpinBox->setValue(99.99);

    connect(precisionSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(changePrecision(int)));

    QVBoxLayout *spinBoxLayout = new QVBoxLayout(doubleSpinBoxesGroup);
    spinBoxLayout->addWidget(precisionLabel);
    spinBoxLayout->addWidget(precisionSpinBox);
    spinBoxLayout->addWidget(doubleLabel);
    spinBoxLayout->addWidget(doubleSpinBox);
    spinBoxLayout->addWidget(scaleLabel);
    spinBoxLayout->addWidget(scaleSpinBox);
    spinBoxLayout->addWidget(priceLabel);
    spinBoxLayout->addWidget(priceSpinBox);
}

void Window::changePrecision(int precision)
{
    doubleSpinBox->setPrecision(precision);
    scaleSpinBox->setPrecision(precision);
    priceSpinBox->setPrecision(precision);
}
