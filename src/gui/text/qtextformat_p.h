#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <private/qobject_p.h>
#include <qvector.h>
#include <qmap.h>

class QTextPieceTable;

class QTextFormatProperty
{
public:
    inline QTextFormatProperty() : type(QTextFormat::Undefined) {}

    inline QTextFormatProperty(bool value) : type(QTextFormat::Bool)
    { data.boolValue = value; }

    inline QTextFormatProperty(int value) : type(QTextFormat::Integer)
    { data.intValue = value; }

    inline QTextFormatProperty(float value) : type(QTextFormat::Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);

    QTextFormatProperty &operator=(const QTextFormatProperty &rhs);
    inline QTextFormatProperty(const QTextFormatProperty &rhs) : type(QTextFormat::Undefined)
    { (*this) = rhs; }


    inline ~QTextFormatProperty()
    { free(); }

    bool operator==(const QTextFormatProperty &rhs) const;

    QTextFormat::PropertyType type;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
        mutable void *ptr;
    } data;

    inline QString stringValue() const
    { return *reinterpret_cast<QString *>(&data.ptr); }

private:
    void free();
};


class QTextFormatPrivate : public QSharedObject
{
public:
    typedef QMap<int, QTextFormatProperty> PropertyMap;

    PropertyMap properties;
    int type;

    inline bool operator==(const QTextFormatPrivate &rhs) const {
        if (type != rhs.type)
            return false;

        return properties == rhs.properties;
    }
};


class Q_GUI_EXPORT QTextFormatCollection
{
public:
    QTextFormatCollection( QTextPieceTable *_pieceTable = 0 ) : pieceTable(_pieceTable) { ref = 0; }
    ~QTextFormatCollection();

    QTextFormatCollection(const QTextFormatCollection &rhs);
    QTextFormatCollection &operator=(const QTextFormatCollection &rhs);


    QTextFormatGroup *createGroup(const QTextFormat &newFormat);
    QTextFormatGroup *group(int groupIndex) const;
    int indexForGroup(QTextFormatGroup *group);

    int indexForFormat(const QTextFormat &f);
    bool hasFormatCached(const QTextFormat &format) const;

    QTextFormat format(int idx) const;

    inline QTextBlockFormat blockFormat(int index) const
    { return format(index).toBlockFormat(); }
    inline QTextCharFormat charFormat(int index) const
    { return format(index).toCharFormat(); }
    inline QTextListFormat listFormat(int index) const
    { return format(index).toListFormat(); }
    inline QTextTableFormat tableFormat(int index) const
    { return format(index).toTableFormat(); }
    inline QTextImageFormat imageFormat(int index) const
    { return format(index).toImageFormat(); }

    inline int numFormats() const { return formats.count(); }

    QTextFormatGroup *createGroup(int index);

    mutable QAtomic ref;

    QTextPieceTable *pieceTable;
private:

    mutable QVector<QSharedDataPointer<QTextFormatPrivate> > formats;
    QVector<QTextFormatGroup *> groups;
};

class QTextFormatGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextFormatGroup);
public:
    QTextFormatCollection *collection;
    int index;

    typedef QList<QTextBlockIterator> BlockList;
    BlockList blocks;
};



#endif // QTEXTFORMAT_P_H
