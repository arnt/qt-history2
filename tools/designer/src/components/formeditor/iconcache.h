#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QMap>
#include <QVariant>
#include <QString>
#include <QPair>
#include <QPixmap>
#include <QIcon>

#include <resourcefile.h>
#include <abstracticoncache.h>
#include "formeditor_global.h"

/*
    We need two caches - one for icons and one for pixmaps - which are in all
    other respects identical.
*/
template <typename Item>
class ResourceCache
{
public:
    Item keyToItem(const QString &path, const QString &resourcePath = QString());
    QString itemToFilePath(const Item &item) const;
    QString itemToQrcPath(const Item &item) const;

    QList<Item> itemList() const;

private:
    typedef QPair<QString, QString> Key;
    typedef QMap<Key, Item> KeyToItemMap;
    typedef QMap<int, Key> SerialToKeyMap;

    KeyToItemMap m_key_to_item;
    SerialToKeyMap m_serial_to_key;
};

template <typename Item>
Item ResourceCache<Item>::keyToItem(const QString &filePath, const QString &qrcPath)
{
    Key key = qMakePair(filePath, qrcPath);
    typename KeyToItemMap::const_iterator it = m_key_to_item.find(key);
    if (it != m_key_to_item.end())
        return *it;

    QString real_path;
    if (!qrcPath.isEmpty()) {
        ResourceFile rf(qrcPath);
        if (rf.load()) {
            real_path = rf.resolvePath(filePath);
        } else {
            qWarning("IconCache::nameToIcon(): failed to open \"%s\": %s",
                        qrcPath.toLatin1().constData(),
                        rf.errorMessage().toLatin1().constData());
        }
    } else {
       real_path = filePath;
    }

    if (real_path.isEmpty())
        return Item();

    Item item(real_path);
    if (item.isNull())
        return Item();
    m_key_to_item.insert(key, item);
    m_serial_to_key.insert(item.serialNumber(), key);

    return item;
}

template <typename Item>
QString ResourceCache<Item>::itemToFilePath(const Item &item) const
{
    typename SerialToKeyMap::const_iterator it = m_serial_to_key.find(item.serialNumber());
    if (it != m_serial_to_key.end())
        return (*it).first;
    return QString();
}

template <typename Item>
QString ResourceCache<Item>::itemToQrcPath(const Item &item) const
{
    typename SerialToKeyMap::const_iterator it = m_serial_to_key.find(item.serialNumber());
    if (it != m_serial_to_key.end())
        return (*it).second;
    return QString();
}

template <typename Item>
QList<Item> ResourceCache<Item>::itemList() const
{
    return m_key_to_item.values();
}

class QT_FORMEDITOR_EXPORT IconCache : public AbstractIconCache
{
    Q_OBJECT
public:
    IconCache(QObject *parent)
        : AbstractIconCache(parent) {}
    virtual QIcon nameToIcon(const QString &path, const QString &resourcePath = QString())
        { return m_icon_cache.keyToItem(path, resourcePath); }
    virtual QString iconToFilePath(const QIcon &pm) const
        { return m_icon_cache.itemToFilePath(pm); }
    virtual QString iconToQrcPath(const QIcon &pm) const
        { return m_icon_cache.itemToQrcPath(pm); }
    virtual QPixmap nameToPixmap(const QString &path, const QString &resourcePath = QString())
        { return m_pixmap_cache.keyToItem(path, resourcePath); }
    virtual QString pixmapToFilePath(const QPixmap &pm) const
        { return m_pixmap_cache.itemToFilePath(pm); }
    virtual QString pixmapToQrcPath(const QPixmap &pm) const
        { return m_pixmap_cache.itemToQrcPath(pm); }

    QList<QPixmap> pixmapList() const
        { return m_pixmap_cache.itemList(); }
    QList<QIcon> iconList() const
        { return m_icon_cache.itemList(); }

private:
    ResourceCache<QIcon> m_icon_cache;
    ResourceCache<QPixmap> m_pixmap_cache;
};

#endif // ICONCACHE_H
