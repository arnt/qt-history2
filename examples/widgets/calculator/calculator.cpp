#include <QtGui>

#include <math.h>

#include "calculator.h"
#include "button.h"

Calculator::Calculator(QWidget *parent)
    : QDialog(parent)
{
    sumSoFar = 0.0;
    pendingTerm = 0.0;
    sumInMemory = 0.0;
    waitingForOperand = true;

    lineEdit = new QLineEdit("0", this);
    lineEdit->setReadOnly(true);
    lineEdit->setAlignment(Qt::AlignRight);
    lineEdit->setMaxLength(15);
    lineEdit->installEventFilter(this);

    QFont font = lineEdit->font();
    font.setPointSize(font.pointSize() + 8);
    lineEdit->setFont(font);

    QColor digitColor(150, 205, 205);
    QColor backspaceColor(225, 185, 135);
    QColor memoryColor(100, 155, 155);
    QColor operatorColor(155, 175, 195);

    for (int i = 0; i < NumDigitButtons; ++i) {
	digitButtons[i] = createButton(QString::number(i), digitColor,
                                       SLOT(digitPressed()));
    }

    pointButton = createButton(tr("."), digitColor, SLOT(pointPressed()));
    signButton = createButton(tr("±"), digitColor, SLOT(signPressed()));
   
    backspaceButton = createButton(tr("Backspace"), backspaceColor,
                                   SLOT(backspacePressed()));
    clearButton = createButton(tr("C"), backspaceColor, SLOT(clear()));
    clearAllButton = createButton(tr("CA"), backspaceColor.light(120),
                                  SLOT(clearAll()));

    clearMemoryButton = createButton(tr("MC"), memoryColor,
                                     SLOT(clearMemory()));
    readMemoryButton = createButton(tr("MR"), memoryColor, SLOT(readMemory()));
    setMemoryButton = createButton(tr("MS"), memoryColor, SLOT(setMemory()));
    addToMemoryButton = createButton(tr("M+"), memoryColor,
                                     SLOT(addToMemory()));

    divisionButton = createButton(tr("÷"), operatorColor,
                                  SLOT(multiplicativeOperatorPressed()));
    timesButton = createButton(tr("×"), operatorColor,
                               SLOT(multiplicativeOperatorPressed()));
    minusButton = createButton(tr("-"), operatorColor,
                               SLOT(additiveOperatorPressed()));
    plusButton = createButton(tr("+"), operatorColor,
                              SLOT(additiveOperatorPressed()));

    squareRootButton = createButton(tr("Sqrt"), operatorColor,
                                    SLOT(unaryOperatorPressed()));
    powerButton = createButton(tr("x²"), operatorColor,
                               SLOT(unaryOperatorPressed()));
    invertButton = createButton(tr("1/x"), operatorColor,
                                SLOT(unaryOperatorPressed()));
    equalButton = createButton(tr("="), operatorColor.light(120),
                               SLOT(equalPressed()));
   
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(lineEdit, 0, 0, 1, 6);
    mainLayout->addWidget(backspaceButton, 1, 0, 1, 2);
    mainLayout->addWidget(clearButton, 1, 2, 1, 2);
    mainLayout->addWidget(clearAllButton, 1, 4, 1, 2);

    mainLayout->addWidget(clearMemoryButton, 2, 0);
    mainLayout->addWidget(readMemoryButton, 3, 0);
    mainLayout->addWidget(setMemoryButton, 4, 0);
    mainLayout->addWidget(addToMemoryButton, 5, 0);

    for (int i = 1; i < NumDigitButtons; ++i) {
        int row = ((9 - i) / 3) + 2;
        int column = ((i - 1) % 3) + 1;
        mainLayout->addWidget(digitButtons[i], row, column);
    }

    mainLayout->addWidget(digitButtons[0], 5, 1);
    mainLayout->addWidget(pointButton, 5, 2);
    mainLayout->addWidget(signButton, 5, 3);

    mainLayout->addWidget(divisionButton, 2, 4);
    mainLayout->addWidget(timesButton, 3, 4);
    mainLayout->addWidget(minusButton, 4, 4);
    mainLayout->addWidget(plusButton, 5, 4);

    mainLayout->addWidget(squareRootButton, 2, 5);
    mainLayout->addWidget(powerButton, 3, 5);
    mainLayout->addWidget(invertButton, 4, 5);
    mainLayout->addWidget(equalButton, 5, 5);

    setWindowTitle(tr("Calculator"));
}

bool Calculator::eventFilter(QObject *target, QEvent *event)
{
    if (target == lineEdit) {
        if (event->type() == QEvent::MouseButtonPress
                || event->type() == QEvent::MouseButtonDblClick
                || event->type() == QEvent::MouseButtonRelease) {
            QPalette palette = lineEdit->palette();
            palette.setColor(QPalette::Base,
                             lineEdit->palette().color(QPalette::Text));
            palette.setColor(QPalette::Text,
                             lineEdit->palette().color(QPalette::Base));
            lineEdit->setPalette(palette);
        }
    }
    return QDialog::eventFilter(target, event);
}

void Calculator::digitPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int digitValue = button->text().toInt();
    if (lineEdit->text() == "0" && digitValue == 0.0)
        return;

    if (waitingForOperand) {
        lineEdit->clear();
	waitingForOperand = false;
    }
    lineEdit->setText(lineEdit->text() + digitButtons[digitValue]->text());
}

void Calculator::unaryOperatorPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    double nextOperand = lineEdit->text().toDouble();
    QString theOperator = button->text();

    if (theOperator == tr("Sqrt")) {
        if (nextOperand < 0.0) {
            abortOperation();
            return;
        }
        sumSoFar = sqrt(nextOperand);
    } else if (theOperator == tr("x²")) {
        sumSoFar = pow(nextOperand, 2.0);
    } else if (theOperator == tr("1/x")) {
        if (nextOperand == 0.0) {
	    abortOperation();
	    return;        
        }
        sumSoFar = 1.0 / nextOperand;
    }
    lineEdit->setText(QString::number(sumSoFar));
    waitingForOperand = true;
}

void Calculator::additiveOperatorPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    double nextOperand = lineEdit->text().toDouble();
    QString theOperator = button->text();

    if (!pendingMultiplicativeOperator.isEmpty()) {
        if (!calculate(nextOperand, pendingMultiplicativeOperator)) {
            abortOperation();
	    return;
        }
        lineEdit->setText(QString::number(pendingTerm));
        nextOperand = pendingTerm;
        pendingMultiplicativeOperator.clear();
    }

    if (!pendingAdditiveOperator.isEmpty()){
        if (!calculate(nextOperand, pendingAdditiveOperator)) {
            abortOperation();
	    return;
        }
        lineEdit->setText(QString::number(sumSoFar));
    } else {
        sumSoFar = nextOperand;
    }

    pendingAdditiveOperator = theOperator;
    waitingForOperand = true;
}

void Calculator::multiplicativeOperatorPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    double nextOperand = lineEdit->text().toDouble();
    QString theOperator = button->text();

    if (!pendingMultiplicativeOperator.isEmpty()) {
        if (!calculate(nextOperand, pendingMultiplicativeOperator)) {
            abortOperation();
	    return;
        }
        lineEdit->setText(QString::number(pendingTerm));
    } else {
        pendingTerm = nextOperand;
    }

    pendingMultiplicativeOperator = theOperator;
    waitingForOperand = true;
}

void Calculator::equalPressed()
{
    double nextOperand = lineEdit->text().toDouble();

    if (!pendingMultiplicativeOperator.isEmpty()) {
        if (!calculate(nextOperand, pendingMultiplicativeOperator)) {
            abortOperation();
	    return;
        }
        nextOperand = pendingTerm;
    }
    if (!pendingAdditiveOperator.isEmpty()) {
        if (!calculate(nextOperand, pendingAdditiveOperator)) {
            abortOperation();
	    return;
        }
    } else {
        sumSoFar = nextOperand;
    }
   
    lineEdit->setText(QString::number(sumSoFar));
    pendingMultiplicativeOperator.clear();
    pendingAdditiveOperator.clear();
    pendingTerm = 0.0;
    sumSoFar = 0.0;
    waitingForOperand = true;
}

void Calculator::pointPressed()
{
  if (!lineEdit->text().contains(".")) {
        lineEdit->setText(lineEdit->text() + tr("."));
        waitingForOperand = false;
  }
}

void Calculator::signPressed()
{
    QString text = lineEdit->text();
    double value = text.toDouble();

    if (value > 0.0) {
        text.prepend(tr("-"));
    } else if (value < 0.0) {
        text.remove(0, 1);
    }
    lineEdit->setText(text);
}

void Calculator::backspacePressed()
{
    if (waitingForOperand)
        return;

    QString text = lineEdit->text();
    text.chop(1);
    if (text.isEmpty()) {
        text = "0";
        waitingForOperand = true;
    }
    lineEdit->setText(text);
}

void Calculator::clear()
{
    lineEdit->setText("0");
    waitingForOperand = true;
}

void Calculator::clearAll()
{
    sumSoFar = 0.0;
    pendingTerm = 0.0;
    pendingAdditiveOperator.clear();
    pendingMultiplicativeOperator.clear();
    lineEdit->setText("0");
    waitingForOperand = true;
}

void Calculator::clearMemory()
{
    sumInMemory = 0.0;
}

void Calculator::readMemory()
{
    lineEdit->setText(QString::number(sumInMemory));
}

void Calculator::setMemory()
{
    sumInMemory = lineEdit->text().toDouble();
    waitingForOperand = true;
}

void Calculator::addToMemory()
{
    sumInMemory += lineEdit->text().toDouble();
    waitingForOperand = true;
}

QToolButton *Calculator::createButton(const QString &text, const QColor &color,
                                      const char *slot)
{
    QToolButton *button = new Button(text, color, this);
    connect(button, SIGNAL(pressed()), this, slot);
    return button;
}

void Calculator::abortOperation()
{
    lineEdit->setText(tr("####"));
    pendingAdditiveOperator.clear();
    pendingMultiplicativeOperator.clear();
    pendingTerm = 0.0;
    sumSoFar = 0.0;
    waitingForOperand = true;
}

bool Calculator::calculate(double nextOperand, const QString &theOperator)
{
    if (theOperator == tr("+")) {
        sumSoFar += nextOperand;
    } else if (theOperator == tr("-")) {
        sumSoFar -= nextOperand;
    } else if (theOperator == tr("×")) {
        pendingTerm *= nextOperand;
    } else if (theOperator == tr("÷")) {
	if (nextOperand == 0.0)
	    return false;       
	pendingTerm /= nextOperand;
    }
    return true;
}
