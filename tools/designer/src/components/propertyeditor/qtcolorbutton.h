#ifndef QTCOLORBUTTON_H
#define QTCOLORBUTTON_H

#include <QToolButton>

class QtColorButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtColorButton(QWidget *parent = 0);
    ~QtColorButton();

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setColor(const QColor &color);
    QColor color() const;

signals:
    void colorChanged(const QColor &color);
protected:
    void paintEvent(QPaintEvent *e);
private:
    class QtColorButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtColorButton)
    Q_DISABLE_COPY(QtColorButton)
    Q_PRIVATE_SLOT(d_func(), void slotEditColor());
};

#endif
