#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include "xmldata.h"

#include <QTextStream>
#include <QList>
#include <QString>

QT_DECLARE_CLASS(QStringList)
QT_DECLARE_CLASS(QSettings)

struct HTMLImage
{
public:
    QString        file;
    QString        generatorName;
    GeneratorFlags flags;
    QString        details;
};

struct HTMLRow
{
public:
    QString testcase;
    QList<HTMLImage> referenceImages;
    QList<HTMLImage> foreignImages;
    QList<HTMLImage> images;
};

struct HTMLSuite
{
public:
    QString name;
    QMap<QString, HTMLRow*> rows;
};

class HTMLGenerator
{
public:
    HTMLGenerator();

    void startSuite(const QString &name);
    void startRow(const QString &testcase);
    void addImage(const QString &generator, const QString &image,
                  const QString &details, GeneratorFlags flags);
    void endRow();
    void endSuite();

    void generateIndex(const QString &file);
    void generatePages();

    void run(int argc, char **argv);

private:
    void generateSuite(const HTMLSuite &suite);

    void generateReferencePage(const HTMLSuite &suite);
    void generateHistoryPages(const HTMLSuite &suite);
    void generateHistoryForEngine(const HTMLSuite &suite, const QString &engine);
    void generateQtComparisonPage(const HTMLSuite &suite);

    void generateHeader(QTextStream &out, const QString &name,
                        const QStringList &generators);
    void startGenerateRow(QTextStream &out, const QString &name);
    void generateImages(QTextStream &out,
                        const QList<HTMLImage> &images);
    void generateHistoryImages(QTextStream &out,
                               const QList<HTMLImage> &images);
    void finishGenerateRow(QTextStream &out, const QString &name);
    void generateFooter(QTextStream &out);

    void processArguments(int argc, char **argv);
    void convertToHtml();
    void createPerformance();
private:
    QMap<QString, HTMLSuite*> suites;
    QMap<QString, XMLEngine*> engines;

    QSettings *settings;
    QString outputDirName;
    QString baseDataDir;
    QString htmlOutputDir;
};

#endif
