#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <qvector.h>
#include <qmap.h>

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
    int inheritedType;

    inline bool operator==(const QTextFormatPrivate &rhs) const {
	if (type != rhs.type || inheritedType != rhs.inheritedType)
	    return false;

	return properties == rhs.properties;
    }
};



class Q_GUI_EXPORT QTextFormatCollection
{
public:
    int createReferenceIndex(const QTextFormat &newFormat);
    QTextFormat updateReferenceIndex(int index, const QTextFormat &newFormat);

    int indexForFormat(const QTextFormat &f);
    bool hasFormatCached(const QTextFormat &format) const;

    QTextFormat format(int idx, int defaultFormatType = -1) const;

    inline QTextBlockFormat blockFormat(int index) const
    { return format(index, QTextFormat::BlockFormat).toBlockFormat(); }
    inline QTextCharFormat charFormat(int index) const
    { return format(index, QTextFormat::CharFormat).toCharFormat(); }
    inline QTextListFormat listFormat(int index) const
    { return format(index, QTextFormat::ListFormat).toListFormat(); }
    inline QTextTableFormat tableFormat(int index) const
    { return format(index, QTextFormat::TableFormat).toTableFormat(); }
    inline QTextImageFormat imageFormat(int index) const
    { return format(index, QTextFormat::ImageFormat).toImageFormat(); }

    inline int numFormats() const { return formats.count(); }

private:
    int indexToReference(int idx) const;
    int referenceToIndex(int ref) const;

    mutable QVector<QSharedDataPointer<QTextFormatPrivate> > formats;
    QVector<int> formatReferences;
};


#endif // QTEXTFORMAT_P_H
