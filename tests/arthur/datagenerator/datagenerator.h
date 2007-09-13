#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include "xmlgenerator.h"
#include "framework.h"

#include <QTextStream>
#include <QSettings>

QT_DECLARE_CLASS(QSvgRenderer)
QT_DECLARE_CLASS(QEngine)
QT_DECLARE_CLASS(QFileInfo)

class DataGenerator
{
public:
    DataGenerator();
    ~DataGenerator();

    void run(int argc, char **argv);
private:
    bool processArguments(int argc, char **argv);
    void testEngines(XMLGenerator &generator, const QString &file,
                     const QString &refUrl);
    void testDirectory(const QString &dirname, const QString &refUrl);
    void testFile(const QString &file, const QString &refUrl,
                  QTextStream &out, QTextStream &hout);
    void testGivenFile();
    void testSuite(XMLGenerator &generator, const QString &suite,
                   const QString &dirName, const QString &refUrl);
    void prepareDirs();

    void testGivenEngines(const QList<QEngine*> engines,
                          const QFileInfo &fileInfo,
                          const QString &file,
                          XMLGenerator &generator,
                          GeneratorFlags flags);
    void testGivenEngines(const QList<QEngine*> engines,
                          const QFileInfo &fileInfo,
                          const QString &file,
                          XMLGenerator &generator,
                          int iterations,
                          GeneratorFlags flags);

    bool wantedEngine(const QString &engine) const;
private:
    QSvgRenderer *renderer;
    Framework settings;

    QString engineName;
    QString suiteName;
    QString testcase;
    QString fileName;
    QString outputDirName;
    QString baseDataDir;
    int     iterations;
};

#endif
