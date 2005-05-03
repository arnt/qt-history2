#ifndef ITEM_H
#define ITEM_H

#include <QtCore/QString>
#include <QtGui/QPixmap>

class Item
{
public:
    enum ItemType { BusinessCard, Weapon, Armor };
    Item(ItemType type, const QString &pixName, const QString &name, const QString &description)
        : m_name(name), m_description(description), m_type(type), m_pic(pixName) {}
    virtual ~Item() {}
    inline QString name() const { return m_name; }
    inline QString description() const { return m_description; }
    inline ItemType type() const { return m_type; }
    inline QString pixmapName() const { return m_pic; }

private:
    QString m_name;
    QString m_description;
    ItemType m_type;
    QString m_pic;
};

class BusinessCard : public Item
{
public:
    BusinessCard(const QString &name, const QString &description, const QString &bigpic)
        : Item(Item::BusinessCard, QLatin1String(":/qthack/images/bcard.png"),
               name, description), m_bigPic(bigpic)
    {}
    inline QString bigPicture() const { return m_bigPic; }
private:
    QString m_bigPic;
};
#endif
