#ifndef SIGNALSANDSLOTS_H
#define SIGNALSANDSLOTS_H

#define Counter PlainCounter

class Counter
{
public:
    Counter() { m_value = 0; }

    int value() const { return m_value; }
    void setValue(int value);

private:
    int m_value;
};

#undef Counter
#define Counter ObjectCounter

#include <QObject>

class Counter : public QObject
{
    Q_OBJECT

public:
    Counter() { m_value = 0; }

    int value() const { return m_value; }

public slots:
    void setValue(int value);

signals:
    void valueChanged(int newValue);

private:
    int m_value;
};

#endif
