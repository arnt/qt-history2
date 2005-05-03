#ifndef CHARACTER_H
#define CHARACTER_H

#include <QtCore/QPoint>
#include <QtCore/QList>

class Item;

class Cursor
{
public:
    Cursor();
    ~Cursor();

    void setPosition(const QPoint &pt, bool countStep = true);
    inline QPoint position() const { return m_pos; }
    inline int totalSteps() const { return m_totalSteps; }

    inline QList<const Item *> items() const { return m_items; }
    void addItem(const Item *item);

private:
    QList<const Item *> m_items;
    QPoint m_pos;
    int m_totalSteps;

};
#endif
