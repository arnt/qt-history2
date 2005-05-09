#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>

class DisplayShape;

class DisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DisplayWidget::DisplayWidget(QWidget *parent = 0);
    void appendShape(DisplayShape *shape);
    void insertShape(int position, DisplayShape *shape);
    QSize minimumSizeHint() const;
    void reset();
    DisplayShape *shape(int index) const;
    int shapesCount() const;

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

signals:
    void displayEmpty();
    void targetReached(DisplayShape *shape);
    void categoryRequested(const QString &name);
    void documentationRequested(const QString &name);
    void exampleRequested(const QString &name);
    void launchRequested(const QString &name);

private slots:
    void updateShapes();

private:
    void startTimer();

    bool empty;
    bool emptying;
    QList<DisplayShape*> shapes;
    QTimer *timer;
};

#endif
