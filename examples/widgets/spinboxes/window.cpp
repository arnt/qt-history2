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

    setWindowTitle(tr("Spin Boxes"));
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

    QLabel *dateLabel = new QLabel(editsGroup);
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate(),
                                                editsGroup);
    dateEdit->setDateRange(QDate(2005, 1, 1), QDate(2010, 12, 31));
    dateLabel->setText(tr("Appointment date (between %0 and %1):")
                       .arg(dateEdit->minimumDate().toString(Qt::ISODate))
                       .arg(dateEdit->maximumDate().toString(Qt::ISODate)));

    QLabel *timeLabel = new QLabel(editsGroup);
    QDateTimeEdit *timeEdit = new QDateTimeEdit(QTime::currentTime(),
                                                editsGroup);
    timeEdit->setTimeRange(QTime(9, 0, 0, 0), QTime(16, 30, 0, 0));
    timeLabel->setText(tr("Appointment time (between %0 and %1):")
                       .arg(timeEdit->minimumTime().toString(Qt::ISODate))
                       .arg(timeEdit->maximumTime().toString(Qt::ISODate)));

    QLabel *formatLabel = new QLabel(tr("Format string for the meeting date "
                                        "and time:"), editsGroup);

    meetingLabel = new QLabel(editsGroup);
    meetingEdit = new QDateTimeEdit(QDateTime::currentDateTime(), editsGroup);

    QComboBox *formatComboBox = new QComboBox(editsGroup);
    formatComboBox->addItem("yyyy-MM-dd hh:mm:ss (zzz ms)");
    formatComboBox->addItem("hh:mm:ss MM/dd/yyyy");
    formatComboBox->addItem("hh:mm:ss dd/MM/yyyy");
    formatComboBox->addItem("hh:mm:ss");
    formatComboBox->addItem("hh:mm ap");

    connect(formatComboBox, SIGNAL(activated(const QString &)),
            this, SLOT(setFormatString(const QString &)));

    setFormatString(formatComboBox->currentText());

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
    meetingEdit->setDisplayFormat(formatString);
    if (meetingEdit->displayedSections() & QDateTimeEdit::DateSections_Mask) {
        meetingEdit->setDateRange(QDate(2004, 11, 1), QDate(2005, 11, 30));
        meetingLabel->setText(tr("Meeting date (between %0 and %1):")
            .arg(meetingEdit->minimumDate().toString(Qt::ISODate))
	    .arg(meetingEdit->maximumDate().toString(Qt::ISODate)));
    } else {
        meetingEdit->setTimeRange(QTime(0, 7, 20, 0), QTime(21, 0, 0, 0));
        meetingLabel->setText(tr("Meeting time (between %0 and %1):")
            .arg(meetingEdit->minimumTime().toString(Qt::ISODate))
	    .arg(meetingEdit->maximumTime().toString(Qt::ISODate)));
    }
}

void Window::createDoubleSpinBoxes()
{
    doubleSpinBoxesGroup = new QGroupBox(tr("Double precision spinboxes"), 
					 this);

    QLabel *precisionLabel = new QLabel(tr("Number of decimal places "
                                           "to show:"),
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

void Window::changePrecision(int decimals)
{
    doubleSpinBox->setDecimals(decimals);
    scaleSpinBox->setDecimals(decimals);
    priceSpinBox->setDecimals(decimals);
}
