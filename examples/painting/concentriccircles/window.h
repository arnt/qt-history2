#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QLabel;
class CircleWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    QLabel *createLabel(const QString &text);

    QLabel *aliasedLabel;
    QLabel *antialiasedLabel;
    QLabel *intLabel;
    QLabel *floatLabel;
    CircleWidget *circleWidgets[2][2];
};

#endif
