#ifndef QTOOLBARBUTTON_H
#define QTOOLBARBUTTON_H

#include <qabstractbutton.h>

class QMenu;
class QToolBarButtonPrivate;

class QToolBarButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QToolBarButton);

public:
    QToolBarButton(QWidget *parent);
    ~QToolBarButton();

    void setMenu(QMenu *menu);
    QMenu *menu() const;
    void showMenu();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    bool hitButton(const QPoint &pos) const;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void actionEvent(QActionEvent *event);
};

#endif // QTOOLBARBUTTON_H
