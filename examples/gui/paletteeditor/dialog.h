#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>

class Widget;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();

private slots:
    void changeColor();

private:
    void updatePalette();
    QPalette::ColorGroup getColorGroup();
    QPalette::ColorRole getColorRole();

    Widget *activeWidget;
    Widget *inactiveWidget;
    Widget *disabledWidget;
    QComboBox *groupComboBox;
    QComboBox *roleComboBox;
    QLabel *colorLabel;
    QColor color;
    QPalette userPalette;
};

#endif
