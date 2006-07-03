#ifndef XMLDATA_H
#define XMLDATA_H

#include <QtXml>

#include <QMap>
#include <QDateTime>

enum GeneratorFlag {
    Normal      = 0x1 << 0,
    Default     = 0x1 << 1,
    Foreign     = 0x1 << 2,
    Reference   = 0x1 << 3
};
Q_DECLARE_FLAGS(GeneratorFlags, GeneratorFlag);

struct XMLData
{
    XMLData()
        : date(QDateTime::currentDateTime()),
          timeToRender(0), iterations(0), maxElapsed(0),
          minElapsed(0)
    {}
    XMLData(const QDateTime &dt, int ttr, int itrs = 1)
        : date(dt), timeToRender(ttr),
          iterations(itrs), maxElapsed(0), minElapsed(0)
    {}
    XMLData(const QString &dt, int ttr, int itrs = 1)
        : timeToRender(ttr), iterations(itrs),
          maxElapsed(0), minElapsed(0)
    {
        date = QDateTime::fromString(dt);
    }
    QDateTime date;
    int timeToRender;
    int iterations;
    QString details;
    int maxElapsed;
    int minElapsed;
};

struct XMLFile
{
    XMLFile()
    {}
    XMLFile(const QString &testcase)
        : name(testcase)
    {}
    XMLFile(const QString &testcase, const QString &img)
        : name(testcase), output(img)
    {}

    QString name;
    QString output;
    QList<XMLData> data;
};

struct XMLSuite
{
    XMLSuite()
    {}
    XMLSuite(const QString &n)
        : name(n)
    {}

    QString name;
    QMap<QString, XMLFile*> files;
};

struct XMLEngine
{
    XMLEngine()
        : defaultEngine(false), foreignEngine(false),
          referenceEngine(false)
    {}
    XMLEngine(const QString &engine, bool def)
        : name(engine), defaultEngine(def), foreignEngine(false),
          referenceEngine(false)
    {}

    QString name;
    bool defaultEngine;
    bool foreignEngine;
    bool referenceEngine;
    QMap<QString, XMLSuite*> suites;
    QDateTime generationDate;
};



class XMLReader : public QXmlDefaultHandler
{
public:
    XMLEngine *xmlEngine() const
    {
        return engine;
    }

    bool startElement(const QString &namespaceURI,
                      const QString &localName,
                      const QString &qName,
                      const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);
    bool fatalError(const QXmlParseException &exception);
private:
    XMLEngine *engine;
    XMLSuite  *suite;
    XMLFile   *file;
};

#endif
