#ifndef WINDOW_H
#define WINDOW_H

#include <QPixmap>
#include <QWidget>

class QLabel;
class QLineEdit;
class QMouseEvent;
class QTextEdit;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QLabel *iconLabel;
    QLineEdit *nameEdit;
    QPixmap iconPixmap;
    QTextEdit *commentEdit;
};

#endif
