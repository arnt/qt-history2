#ifndef ECHODIALOG_H
#define ECHODIALOG_H

#include <QWidget>

#include "echointerface.h"

class QString;
class QLineEdit;
class QLabel;
class QPushButton;
class QGridLayout;

class EchoWindow : public QWidget
{
    Q_OBJECT

public:
    EchoWindow();

private slots:
    void sendEcho();
    
private: 
    void createGUI();
    bool loadPlugin();

    EchoInterface *echoInterface;
    QLineEdit *lineEdit;
    QLabel *label;
    QPushButton *button;
    QGridLayout *layout;
};

#endif
