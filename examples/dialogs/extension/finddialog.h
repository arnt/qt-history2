#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(QWidget *parent = 0);

private:
    QLabel *label;
    QLineEdit *lineEdit;
    QCheckBox *caseCheckBox;
    QCheckBox *fromStartCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QCheckBox *searchSelectionCheckBox;
    QCheckBox *backwardCheckBox;
    QPushButton *findButton;
    QPushButton *closeButton;
    QPushButton *moreButton;
    QWidget *extension;
};

#endif
