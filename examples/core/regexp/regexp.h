#ifndef REGEXP_H
#define REGEXP_H

#include <QtGui>

class Regexp : public QDialog
{
    Q_OBJECT

public:
    Regexp(QWidget* parent = 0);

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
    QStatusBar *statusBar;

public slots:
    void copy();
    void execute();

private:
    void languageChange();
};

#endif
