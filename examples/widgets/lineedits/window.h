#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QComboBox;
class QLineEdit;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void slotEchoChanged( int );
    void slotValidatorChanged( int );
    void slotAlignmentChanged( int );
    void slotInputMaskChanged( int );
    void slotAccessChanged( int );

private:
    QLineEdit *echoLineEdit;
    QLineEdit *validatorLineEdit;
    QLineEdit *alignmentLineEdit;
    QLineEdit *inputMaskLineEdit;
    QLineEdit *accessLineEdit;
};

#endif
