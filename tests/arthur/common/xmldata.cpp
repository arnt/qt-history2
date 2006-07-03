#include "xmldata.h"


bool XMLReader::startElement(const QString &, const QString &localName,
                             const QString &, const QXmlAttributes &attributes)
{
    if (localName == "arthur" ) {
        QString engineName   = attributes.value("engine");
        QString defaultStr   = attributes.value("default");
        QString foreignStr   = attributes.value("foreign");
        QString referenceStr = attributes.value("reference");
        QString genDate      = attributes.value("generationDate");
        engine = new XMLEngine(engineName, defaultStr == "true");
        engine->foreignEngine   = (foreignStr == "true");
        engine->referenceEngine = (referenceStr == "true");
        if (!genDate.isEmpty())
            engine->generationDate  = QDateTime::fromString(genDate);
        else
            engine->generationDate  = QDateTime::currentDateTime();
    } else if (localName == "suite") {
        QString suiteName = attributes.value("dir");
        suite = new XMLSuite(suiteName);
    } else if (localName == "file") {
        QString testName = attributes.value("name");
        QString outputName = attributes.value("output");
        file = new XMLFile(testName, outputName);
    } else if (localName == "data") {
        QString dateStr = attributes.value("date");
        QString timeStr = attributes.value("time_to_render");
        QString itrStr = attributes.value("iterations");
        QString detailsStr = attributes.value("details");
        QString maxElapsedStr = attributes.value("maxElapsed");
        QString minElapsedStr = attributes.value("minElapsed");
        XMLData data(dateStr, timeStr.toInt(),
                     (!itrStr.isEmpty())?itrStr.toInt():1);
        data.details = detailsStr;
        if (maxElapsedStr.isEmpty())
            data.maxElapsed = data.timeToRender;
        else
            data.maxElapsed = maxElapsedStr.toInt();
        if (minElapsedStr.isEmpty())
            data.minElapsed = data.timeToRender;
        else
            data.minElapsed = minElapsedStr.toInt();

        file->data.append(data);
    } else {
        qDebug()<<"Error while parsing element :"<<localName;
        return false;
    }
    return true;
}

bool XMLReader::endElement(const QString &, const QString &localName,
                           const QString &)
{
    if (localName == "arthur" ) {
        //qDebug()<<"done";
    } else if (localName == "suite") {
        engine->suites.insert(suite->name, suite);
    } else if (localName == "file") {
        suite->files.insert(file->name, file);
    }
    return true;
}

bool XMLReader::fatalError(const QXmlParseException &)
{
    return true;
}
