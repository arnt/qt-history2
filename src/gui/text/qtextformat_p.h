#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <private/qobject_p.h>
#include <qvector.h>
#include <qmap.h>
#include <qrect.h>

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

QDataStream &operator<<(QDataStream &stream, const QTextFormatProperty &prop);
QDataStream &operator>>(QDataStream &stream, QTextFormatProperty &prop);

class QTextFormatPrivate : public QSharedData
{
public:
    // keep Q_INT* types here, so we can safely stream to a datastream
    typedef QMap<Q_INT32, QTextFormatProperty> PropertyMap;

    PropertyMap properties;
    Q_INT32 type;

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


    QTextFormatObject *createObject(const QTextFormat &newFormat);
    QTextFormatObject *object(int objectIndex) const;
    int indexForObject(QTextFormatObject *object);

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

    QTextFormatObject *createObject(int index);

    mutable QAtomic ref;

    QTextPieceTable *pieceTable;
    const QVector<QTextFormatObject *> &objects() const { return objs; }
private:

    mutable QVector<QSharedDataPointer<QTextFormatPrivate> > formats;
    QVector<QTextFormatObject *> objs;
};

class QTextFormatObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextFormatObject)
public:
    QTextPieceTable *pieceTable() const { return collection->pieceTable; }
    QTextFormatCollection *collection;
    int index;
};

class QTextBlockGroupPrivate : public QTextFormatObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextBlockGroup)
public:

    typedef QList<QTextBlockIterator> BlockList;
    BlockList blocks;
};

class QTextFramePrivate : public QTextFormatObjectPrivate
{
    friend class QTextPieceTable;
    Q_DECLARE_PUBLIC(QTextFrame)
public:

    virtual void fragmentAdded(const QChar &type, uint fragment);
    virtual void fragmentRemoved(const QChar &type, uint fragment);
    void remove_me();

    uint fragment_start;
    uint fragment_end;

    QTextFrame *parentFrame;
    QList<QTextFrame *> childFrames;
    QRect rect;
};


#endif // QTEXTFORMAT_P_H
