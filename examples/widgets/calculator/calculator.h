#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDialog>

class QLineEdit;
class QToolButton;

class Calculator : public QDialog
{
    Q_OBJECT

public:
    Calculator(QWidget *parent = 0);

protected:
    bool eventFilter(QObject *target, QEvent *event);

private slots:
    void digitPressed();
    void unaryOperatorPressed();
    void additiveOperatorPressed();
    void multiplicativeOperatorPressed();
    void equalPressed();
    void pointPressed();
    void changeSignPressed();
    void backspacePressed();
    void clear();
    void clearAll();
    void clearMemory();
    void readMemory();
    void setMemory();
    void addToMemory();

private:
    QToolButton *createButton(const QString &text, const QColor &color,
                              const char *slot);
    void abortOperation();
    bool calculate(double nextOperand, const QString &theOperator);

    double sumSoFar;
    double pendingTerm;
    double sumInMemory;
    bool waitingForOperand;
    QString pendingAdditiveOperator;
    QString pendingMultiplicativeOperator;

    QLineEdit *lineEdit;

    enum { NumDigitButtons = 10 };
    QToolButton *digitButtons[NumDigitButtons];

    QToolButton *pointButton;
    QToolButton *changeSignButton;
    QToolButton *backspaceButton;
    QToolButton *clearButton;
    QToolButton *clearAllButton;
    QToolButton *clearMemoryButton;
    QToolButton *readMemoryButton;
    QToolButton *setMemoryButton;
    QToolButton *addToMemoryButton;
   
    QToolButton *divisionButton;
    QToolButton *timesButton;
    QToolButton *minusButton;
    QToolButton *plusButton;
    QToolButton *squareRootButton;
    QToolButton *powerButton;
    QToolButton *reciprocalButton;
    QToolButton *equalButton;
};

#endif
