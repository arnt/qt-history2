#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QWidget>

class QPushButton;
class QTextEdit;
class QLineEdit;

class TextFinder : public QWidget
{
    Q_OBJECT

public:
    TextFinder(QWidget *parent = 0);

private slots:
    void on_findButton_clicked();
    
private:
    QWidget* loadUiFile();
    void loadTextFile();

    QPushButton *ui_findButton;
    QTextEdit *ui_textEdit;
    QLineEdit *ui_lineEdit;
    bool isFirstTime;
};

#endif