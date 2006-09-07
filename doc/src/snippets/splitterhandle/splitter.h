#ifndef SPLITTER_H
#define SPLITTER_H

#include <QLinearGradient>
#include <QSplitter>
#include <QSplitterHandle>

class QPaintEvent;

class Splitter : public QSplitter
{
public:
    Splitter(Qt::Orientation orientation, QWidget *parent = 0);

protected:
    QSplitterHandle *createHandle();
};

class SplitterHandle : public QSplitterHandle
{
public:
    SplitterHandle(Qt::Orientation orientation, QSplitter *parent);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QLinearGradient gradient;
};

#endif
