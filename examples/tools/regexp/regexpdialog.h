#ifndef REGEXPDIALOG_H
#define REGEXPDIALOG_H

#include <QtGui>

class RegExpDialog : public QDialog
{
    Q_OBJECT

public:
    RegExpDialog(QWidget *parent = 0);

private slots:
    void refresh();
    void copy();

private:
    QLabel *patternLabel;
    QComboBox *patternComboBox;
    QLabel *textLabel;
    QComboBox *textComboBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalCheckBox;
    QCheckBox *wildcardCheckBox;
    QPushButton *copyButton;
    QPushButton *quitButton;

    QLabel *indexLabel;
    QLineEdit *indexEdit;
    QLabel *matchedLengthLabel;
    QLineEdit *matchedLengthEdit;

    enum { MaxCaptures = 10 };
    QLabel *captureLabels[MaxCaptures];
    QLineEdit *captureEdits[MaxCaptures];
};

#endif
