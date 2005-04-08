#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    selectedDate = QDate::currentDate();
    fontSize = 10;

    QWidget *centralWidget = new QWidget(this);
    QLabel *dateLabel = new QLabel(tr("Date:"), centralWidget);
    QComboBox *monthCombo = new QComboBox(centralWidget);

    for (int month = 1; month <= 12; ++month)
        monthCombo->addItem(QDate::longMonthName(month));

    monthCombo->setCurrentIndex(selectedDate.month() - 1);

    QDateTimeEdit *yearEdit = new QDateTimeEdit(centralWidget);
    yearEdit->setDisplayFormat("yyyy");
    yearEdit->setDateRange(QDate(1753, 1, 1), QDate(8000, 1, 1));
    yearEdit->setDate(selectedDate);

    QLabel *fontSizeLabel = new QLabel(tr("Font size:"), centralWidget);
    QSpinBox *fontSizeSpinBox = new QSpinBox(centralWidget);
    fontSizeSpinBox->setRange(1, 64);
    fontSizeSpinBox->setValue(10);

    editor = new QTextBrowser(centralWidget);
    insertCalendar();

    connect(monthCombo, SIGNAL(activated(int)), this, SLOT(setMonth(int)));
    connect(yearEdit, SIGNAL(dateChanged(QDate)), this, SLOT(setYear(QDate)));
    connect(fontSizeSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setFontSize(int)));

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(dateLabel);
    controlsLayout->addWidget(monthCombo);
    controlsLayout->addWidget(yearEdit);
    controlsLayout->addSpacing(24);
    controlsLayout->addWidget(fontSizeLabel);
    controlsLayout->addWidget(fontSizeSpinBox);
    controlsLayout->addStretch(1);

    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addLayout(controlsLayout);
    centralLayout->addWidget(editor, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::insertCalendar()
{
    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start); 
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignHCenter);
    tableFormat.setBackground(QColor("#e0e0e0"));
    tableFormat.setCellPadding(2);
    tableFormat.setCellSpacing(4);
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14)
                << QTextLength(QTextLength::PercentageLength, 14);
    tableFormat.setColumnWidthConstraints(constraints);

    QTextTable *table = cursor.insertTable(1, 7, tableFormat);

    QTextFrame *frame = cursor.currentFrame();
    QTextFrameFormat frameFormat = frame->frameFormat();
    frameFormat.setBorder(1);
    frame->setFrameFormat(frameFormat);

    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(fontSize);
    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);

    for (int weekDay = 1; weekDay <= 7; ++weekDay) {
        QTextTableCell cell = table->cellAt(0, weekDay-1);
        cursor.setPosition(cell.firstCursorPosition().position());
        cursor.insertText(QString("%1").arg(QDate::longDayName(weekDay)),
                          boldFormat);
    }

    QDate date(selectedDate.year(), selectedDate.month(), 1);
    boldFormat.setBackground(Qt::yellow);
    table->insertRows(table->rows(), 1);

    while (date.month() == selectedDate.month()) {
        int weekDay = date.dayOfWeek();
        QTextTableCell cell = table->cellAt(table->rows()-1, weekDay-1);
        cursor.setPosition(cell.firstCursorPosition().position());

        if (date == QDate::currentDate())
            cursor.insertText(QString("%1").arg(date.day()), boldFormat);
        else
            cursor.insertText(QString("%1").arg(date.day()), format);

        date = date.addDays(1);
        if (weekDay == 7 && date.month() == selectedDate.month())
            table->insertRows(table->rows(), 1);
    }

    setWindowTitle(tr("Calendar for %1 %2").arg(
        QDate::longMonthName(selectedDate.month())).arg(
        selectedDate.year()));
}

void MainWindow::setFontSize(int size)
{
    fontSize = size;
    insertCalendar();
}

void MainWindow::setMonth(int month)
{
    selectedDate = QDate(selectedDate.year(), month + 1, selectedDate.day());
    insertCalendar();
}

void MainWindow::setYear(QDate date)
{
    selectedDate = QDate(date.year(), selectedDate.month(), selectedDate.day());
    insertCalendar();
}
