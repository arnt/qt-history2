#include "widget.h"

Widget::Widget(const QString &caption, QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *radioGroupBox = new QGroupBox("Radio group", this);
    QVBoxLayout *radioLayout = new QVBoxLayout(radioGroupBox);
    QButtonGroup *radioGroup= new QButtonGroup(this);
    for (int i = 1; i <= 3; i++) {
        QRadioButton *button = new QRadioButton("Radio #" + QString::number(i),
                                                this);
        radioGroup->addButton(button);
        radioLayout->addWidget(button);
        if (i == 1)
            button->setChecked(true);
    }
    QGroupBox *checkGroupBox = new QGroupBox("Checkbox group", this);
    QVBoxLayout *checkLayout = new QVBoxLayout(checkGroupBox);
    QButtonGroup *checkGroup= new QButtonGroup(this);
    checkGroup->setExclusive(false);
    for (int i = 1; i <= 2; i++) {
        QCheckBox *button = new QCheckBox("Checkbox #" + QString::number(i),
                                          this);
        checkGroup->addButton(button);
        checkLayout->addWidget(button);
        if (i == 1)
            button->setState(QCheckBox::On);
    }
    QProgressBar *progressBar = new QProgressBar(100, this);
    progressBar->setProgress(63);
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->append("First line of text.");
    textEdit->append("A <a href=\"http://www.trolltech.com\">hyperlink</a>.");
    QLineEdit *lineEdit = new QLineEdit("Line editor", this);
    QComboBox *comboBox = new QComboBox(this);
    comboBox->insertItem("Combo item #1");
    comboBox->insertItem("Combo item #2");
    comboBox->insertItem("Combo item #3");
    comboBox->insertItem("Combo item #4");
    comboBox->setCurrentItem(1);
    QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(-50, 50);
    spinBox->setValue(34);
    spinBox->setPrecision(2);
    spinBox->setPrefix("$ ");
    QPushButton *button = new QPushButton("&Button", this);
    QScrollBar *scrollBar = new QScrollBar(Qt::Horizontal, this);
    scrollBar->setRange(0, 100);
    scrollBar->setSliderPosition(83);
    QSlider *slider = new QSlider(Qt::Horizontal, this);
    slider->setTickmarks(QSlider::TickMarksBelow);
    slider->setRange(0, 100);
    slider->setSliderPosition(71);
    QListWidget *listWidget = new QListWidget(this);
    listWidget->setText(0, "http://www.trolltech.com");
    listWidget->setText(1, "http://doc.trolltech.com");
    listWidget->setText(2, "http://doc.trolltech.com/qq/");
    // TODO Set current item

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(radioGroupBox);
    leftLayout->addWidget(checkGroupBox);
    leftLayout->addWidget(progressBar);
    leftLayout->addWidget(textEdit);
    leftLayout->addStretch(1);
    mainLayout->addLayout(leftLayout);
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(lineEdit);
    rightLayout->addWidget(comboBox);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(spinBox);
    hbox->addWidget(button);
    rightLayout->addLayout(hbox);
    rightLayout->addWidget(scrollBar);
    rightLayout->addWidget(slider);
    rightLayout->addWidget(listWidget);
    mainLayout->addLayout(rightLayout);

    setWindowTitle(caption);
}
