#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QLabel;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    QLabel *createLabel(const QString &text);
};

#endif
