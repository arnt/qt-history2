#include "shower.h"

#include "qengines.h"

#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QPaintEvent>
#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>
#include <QtDebug>

static QString loadFile(const QString &name)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) {
        qDebug("Can't open file '%s'", qPrintable(name));
        return QString();
    }
    QTextStream str(&file);
    return str.readAll();
}

Shower::Shower(const QString &file,
               const QString &engineName)
    : QWidget(0)
{
    foreach(QEngine *qengine, QtEngines::self()->engines()) {
        if (qengine->name() == engineName) {
            engine = qengine;
            break;
        }
    }

    QFileInfo fi(file);
    baseDataDir = fi.absolutePath();
    if (file.endsWith("svg")) {
        renderer = new QSvgRenderer(this);
        renderer->load(file);
    } else {
        qps = QFileInfo(file);
        QString script = loadFile(qps.absoluteFilePath());
        qpsScript = script.split("\n", QString::SkipEmptyParts);
        renderer = 0;
        if (qpsScript.isEmpty()) {
            printf("failed to read file: '%s'\n", qPrintable(qps.fileName()));
            return;
        }
    }
}


QSize Shower::sizeHint() const
{
    return QSize(600, 600);
}


void Shower::paintEvent(QPaintEvent *)
{
    if (buffer.size() != size()) {
        buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        QPainter p(&buffer);
        p.setViewport(0, 0, width(), height());
        p.eraseRect(0, 0, width(), height());
        engine->prepare(size());
        if (renderer) {
            engine->render(renderer, QString("sample"));
        } else {
            engine->render(qpsScript, qps.absoluteFilePath());
        }
        if (!engine->drawOnPainter(&p)) {
            QString tempFileName = QString("%1sample.png").arg(QDir::tempPath());
            engine->save(tempFileName);
            QImage img(tempFileName);
            engine->cleanup();
            QFile::remove(tempFileName);
            p.drawImage(0, 0, img);
        }
    }
    QPainter pt(this);
    pt.drawImage(0, 0, buffer);
}
