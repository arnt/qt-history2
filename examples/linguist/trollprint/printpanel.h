#ifndef PRINTPANEL_H
#define PRINTPANEL_H

#include <QWidget>

class QGroupBox;
class QRadioButton;

class PrintPanel : public QWidget
{
    Q_OBJECT

public:
    PrintPanel(QWidget *parent = 0);

private:
    QGroupBox *twoSidedGroupBox;
    QGroupBox *colorsGroupBox;
    QRadioButton *twoSidedEnabledRadio;
    QRadioButton *twoSidedDisabledRadio;
    QRadioButton *colorsEnabledRadio;
    QRadioButton *colorsDisabledRadio;
};

#endif
