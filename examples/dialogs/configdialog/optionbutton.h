#ifndef OPTIONBUTTON_H
#define OPTIONBUTTON_H

#include <QAbstractButton>
#include <QSize>

class QPaintEvent;
class QWidget;

class OptionButton : public QAbstractButton
{
    Q_OBJECT

public:
    OptionButton(int width, int height, int page, QWidget *parent);
    QSize sizeHint() const;
    int page();

protected:
    void paintEvent(QPaintEvent *event);

private:
    int height;
    int pageNumber;
    int width;
};

#endif
