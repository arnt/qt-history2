#ifndef QRTFORMAT_P_H
#define QRTFORMAT_P_H

#include <private/qobject_p.h>
#include <qdebug.h>

enum { UndefinedIndex = -1 };

// ### use a placement new for bytearray/string
class QTextFormatProperty
{
public:
    enum Type {
	Undefined,
	Boolean,
	Integer,
	Float,
	String,
	Binary
    };

    QTextFormatProperty() : type(Undefined) {}

    QTextFormatProperty(bool value) : type(Boolean)
    { data.boolValue = value; }

    QTextFormatProperty(int value) : type(Integer)
    { data.intValue = value; }

    QTextFormatProperty(float value) : type(Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);
    QTextFormatProperty(const QByteArray &value);

    QTextFormatProperty(const QTextFormatProperty &rhs) : type(Undefined)
    { (*this) = rhs; }

    QTextFormatProperty &operator=(const QTextFormatProperty &rhs);

    ~QTextFormatProperty()
    { free(); }

    bool isValid() const { return type != Undefined; }

    bool operator==(const QTextFormatProperty &rhs) const;
    bool operator!=(const QTextFormatProperty &rhs) const
    { return !operator==(rhs); }

    Type type;
    union {
	bool boolValue;
	int intValue;
	float floatValue;
	void *ptr;
    } data;

    QString stringValue() const
    { return *static_cast<QString *>(data.ptr); }
    QByteArray binaryValue() const
    { return *static_cast<QByteArray *>(data.ptr); }

private:
    void assign( const QTextFormatProperty &rhs);
    void free();
};

#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &debug, const QTextFormatProperty &property)
{
    switch (property.type) {
	case QTextFormatProperty::Undefined: debug << "[Undefined]"; break;
	case QTextFormatProperty::Boolean: debug << "[" << "Boolean:" << property.data.boolValue << "]"; break;
	case QTextFormatProperty::Integer: debug << "[" << "Integer:" << property.data.intValue << "]"; break;
	case QTextFormatProperty::Float: debug << "[" << "Float:" << property.data.floatValue << "]"; break;
	case QTextFormatProperty::String: debug << "[" << "String:" << property.stringValue() << "]"; break;
	case QTextFormatProperty::Binary: debug << "[" << "Binary:" << property.binaryValue() << "]"; break;
	default: Q_ASSERT(false);
    }
    return debug;
}
#endif

typedef QMap<int, QTextFormatProperty> QTextFormatPropertyMap;

class QTextFormatPrivate
{
public:
    QTextFormatPrivate(int _type = -1, int _inheritedType = -1) : type(_type), inheritedType(_inheritedType) {}

    int type;
    int inheritedType;

    bool isValid() const { return type != -1; }

    void clearProperty(int property)
    { properties.remove(property); }
    void setProperty(int propertyId, const QTextFormatProperty &property)
    { properties[propertyId] = property; }

    const QTextFormatProperty property(int propertyId, QTextFormatProperty::Type propType) const;

    bool operator==(const QTextFormatPrivate &rhs) const;

    QTextFormatPropertyMap propertyMap() const { return properties; }

private:
    QTextFormatPropertyMap properties;
};

class QTextFormatCollectionPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QTextFormatCollection);
public:
    int indexToReference(int idx) const;
    int referenceToIndex(int ref) const;

    QVector<QTextFormatPrivate> formats;
    QVector<int> formatReferences;
};

#endif // QRTFORMAT_P_H
