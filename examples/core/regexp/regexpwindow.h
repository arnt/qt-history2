#ifndef REGEXPWINDOW_H
#define REGEXPWINDOW_H

#include <QtGui>

class RegExpWindow : public QMainWindow
{
    Q_OBJECT

public:
    RegExpWindow(QWidget *parent = 0);

private slots:
    void refresh();
    void copy();

private:
    QLabel *regexLabel;
    QComboBox *regexComboBox;
    QLabel *textLabel;
    QComboBox *textComboBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalCheckBox;
    QCheckBox *wildcardCheckBox;
    QLabel *resultLabel;
    QPushButton *copyPushButton;
    QPushButton *quitPushButton;
};

#endif
