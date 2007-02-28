#ifndef SIMPLESTYLE_H
#define SIMPLESTYLE_H

#include <QWindowsStyle>

class QPalette;

class SimpleStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    SimpleStyle() {};

    void polish(QPalette &palette);
};

#endif
