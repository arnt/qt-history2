#ifndef QTSIMPLEXML_H
#define QTSIMPLEXML_H
#include <QString>
#include <q3dict.h>
#include <q3ptrlist.h>
#include <qiodevice.h>
#include <QDomNode>

class QDomDocument;
class QDomElement;

class QtSimpleXml
{
public:
    QtSimpleXml(const QString &name = QString());

    QString name() const;
    QString text() const;

    bool isValid() const;

    QtSimpleXml &operator [](int index);
    QtSimpleXml &operator [](const QString &key);
    QtSimpleXml &operator =(const QString &text);

    void setAttribute(const QString &key, const QString &value);
    QString attribute(const QString &key);

    bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    QString errorString() const;

    QDomDocument toDomDocument() const;
    QDomElement toDomElement(QDomDocument *doc) const;
private:
    void parse(QDomNode node);

    QtSimpleXml *parent;
    Q3Dict<QtSimpleXml> siblStruct;
    Q3PtrList<QtSimpleXml> siblArray;
    Q3Dict<QString> attr;

    QString s;
    QString n;
    bool valid;

    QString errorStr;
};

#endif
