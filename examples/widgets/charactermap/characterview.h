#ifndef CHARACTERVIEW_H
#define CHARACTERVIEW_H

#include <QCursor>
#include <QPoint>
#include <QWidgetView>

class QMouseEvent;

class CharacterView : public QWidgetView
{
    Q_OBJECT

public:
    CharacterView(QWidget *parent = 0);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QCursor dragCursor;
    QPoint dragPosition;
    bool dragging;
};

#endif
