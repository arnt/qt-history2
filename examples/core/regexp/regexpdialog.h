#ifndef REGEXPDIALOG_H
#define REGEXPDIALOG_H

#include <QtGui>

class RegExpWindow : public QMainWindow
{
    Q_OBJECT

public:
    RegExpWindow(QWidget *parent = 0);

private slots:
    void copy();
    void executeRegExp();

private:
    QLabel *regexLabel;
    QComboBox *regexComboBox;
    QLabel *textLabel;
    QComboBox *textComboBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalCheckBox;
    QCheckBox *wildcardCheckBox;
    QLabel *resultLabel;
    QPushButton *executePushButton;
    QPushButton *copyPushButton;
    QPushButton *quitPushButton;
};

#endif
