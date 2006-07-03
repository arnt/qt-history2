#include <QApplication>
#include <QtDebug>

#include "shower.h"
#include "qengines.h"

static void usage()
{
    qDebug()<<"shower <-engine engineName> file";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QString engine = "Raster";
    QString file;
    for (int i = 1; i < argc; ++i) {
        QString opt = argv[i];
        if (opt == "-engine") {
            ++i;
            engine = QString(argv[i]);
        } else if (opt.startsWith('-')) {
            qDebug()<<"Unsupported option "<<opt;
        } else
            file = QString(argv[i]);
    }

    bool engineExists = false;
    QStringList engineNames;
    foreach(QEngine *qengine, QtEngines::self()->engines()) {
        if (qengine->name() == engine) {
            engineExists = true;
        }
        engineNames.append(qengine->name());
    }

    if (file.isEmpty() || engine.isEmpty()) {
        usage();
        return 1;
    }

    if (!engineExists) {
        qDebug()<<"Engine "<<engine<<" doesn't exist!\n"
                <<"Available engines: "<<engineNames;
        usage();
        return 1;
    }
    if (!QFile::exists(file)) {
        qDebug()<<"Specified file "<<file<<" doesn't exist!";
        return 1;
    }

    qDebug()<<"Using engine: "<<engine;
    Shower shower(file, engine);
    shower.show();

    return app.exec();
}
