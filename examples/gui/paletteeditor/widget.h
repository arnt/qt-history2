#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui>

class Widget : public QWidget
{
public:
    Widget(const QString &caption, QWidget *parent);
};

#endif
