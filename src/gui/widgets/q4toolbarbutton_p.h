#ifndef Q4TOOLBUTTON_H
#define Q4TOOLBUTTON_H

#include <qabstractbutton.h>

class QMenu;
class Q4ToolBarButtonPrivate;

class Q4ToolBarButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4ToolBarButton);

public:
    Q4ToolBarButton(QWidget *parent);
    ~Q4ToolBarButton();

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

#endif // Q4TOOLBUTTON_H
