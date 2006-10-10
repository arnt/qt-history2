#include "qengines.h"
#include "paintcommands.h"

#include <QProcess>
#include <QPainter>
#include <QSvgRenderer>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QPrintEngine>

QEngine::~QEngine()
{
}

Q_GLOBAL_STATIC(QtEngines, qtengines_global)
QtEngines * QtEngines::self()
{
    return qtengines_global();
}


QList<QEngine*> QtEngines::engines() const
{
    return m_engines;
}


QList<QEngine*> QtEngines::foreignEngines() const
{
    return m_foreignEngines;
}


QEngine * QtEngines::defaultEngine() const
{
    return m_defaultEngine;
}


QtEngines::QtEngines()
{
    init();
}


void QtEngines::init()
{
    m_defaultEngine = new RasterEngine;
    m_engines << m_defaultEngine
              << new NativeEngine
#if defined(BUILD_OPENGL)
              << new GLEngine
#endif
              << new PDFEngine
#ifdef Q_WS_X11
              << new PSEngine
#endif
#ifdef Q_WS_WIN
			  << new WinPrintEngine
#endif
        ;

    m_foreignEngines << new RSVGEngine;
}

RasterEngine::RasterEngine()
{

}

QString RasterEngine::name() const
{
    return QLatin1String("Raster");
}


void RasterEngine::prepare(const QSize &size)
{
    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0xffffffff);
}


void RasterEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter p(&image);
    r->render(&p);
    p.end();
}


void RasterEngine::render(const QStringList &qpsScript,
                          const QString &absFilePath)
{
    QPainter pt(&image);
    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(&pt);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    pt.end();
}

bool RasterEngine::drawOnPainter(QPainter *p)
{
    p->drawImage(0, 0, image);
    return true;
}

void RasterEngine::save(const QString &file)
{
    image.save(file, "PNG");
}


NativeEngine::NativeEngine()
{

}


QString NativeEngine::name() const
{
#ifdef Q_WS_X11
#ifndef QT_NO_XRENDER
    return QLatin1String("NativeXRender");
#else
    return QLatin1String("NativeXLib");
#endif
#elif (defined Q_WS_WIN32)
    return QLatin1String("NativeWin32");
#elif (defined Q_WS_MAC)
    return QLatin1String("NativeMac");
#elif defined(Q_WS_QWS)
    return QLatin1String("NativeEmbedded");
#endif
}


void NativeEngine::prepare(const QSize &size)
{
    pixmap = QPixmap(size);
    pixmap.fill(Qt::white);
}


void NativeEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter p(&pixmap);
    r->render(&p);
    p.end();
}


void NativeEngine::render(const QStringList &qpsScript,
                          const QString &absFilePath)
{
    QPainter pt(&pixmap);
    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(&pt);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    pt.end();
}

bool NativeEngine::drawOnPainter(QPainter *p)
{
    p->drawPixmap(0, 0, pixmap);
    return true;
}

void NativeEngine::save(const QString &file)
{
    pixmap.save(file, "PNG");
}


#if defined(BUILD_OPENGL)
GLEngine::GLEngine()
    : pbuffer(0), widget(0)
{
    usePixelBuffers = QGLPixelBuffer::hasOpenGLPbuffers();
}


QString GLEngine::name() const
{
    return QLatin1String("OpenGL");
}


void GLEngine::prepare(const QSize &_size)
{
    size = _size;
    if (usePixelBuffers) {
#if (QT_VERSION < 0x040200) && defined(Q_WS_MAC)
        pbuffer = new QGLPixelBuffer(QSize(512, 512), QGLFormat(QGL::SampleBuffers));
#else
        pbuffer = new QGLPixelBuffer(size, QGLFormat(QGL::SampleBuffers));
#endif
    } else {
        widget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    }
}

void GLEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter *p;
    if (usePixelBuffers)
        p = new QPainter(pbuffer);
    else
        p = new QPainter(widget);
    p->fillRect(0, 0, size.width(), size.height(), Qt::white);
    r->render(p);
    p->end();
}

void GLEngine::render(const QStringList &qpsScript,
                      const QString &absFilePath)
{
    QPainter *p;
    if (usePixelBuffers)
        p = new QPainter(pbuffer);
    else
        p = new QPainter(widget);

    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(p);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    p->end();
}

bool GLEngine::drawOnPainter(QPainter *p)
{
    if (usePixelBuffers) {
        QImage img = pbuffer->toImage();
        p->drawImage(0, 0, img);
    } else {
        QPixmap pix = widget->renderPixmap(size.width(), size.height());
        p->drawPixmap(0, 0, pix);
    }
    return true;
}


void GLEngine::save(const QString &file)
{
    if (usePixelBuffers) {
        QImage img = pbuffer->toImage();
        img.save(file, "PNG");
    } else {
        QImage img = widget->renderPixmap(size.width(), size.height()).toImage();
        img.save(file, "PNG");
    }
}

void GLEngine::cleanup()
{
    delete pbuffer;
    delete widget;
}

#endif


PDFEngine::PDFEngine()
{
}


QString PDFEngine::name() const
{
    return QLatin1String("PDF");
}

void PDFEngine::prepare(const QSize &size)
{
    static int i = 1;

    m_size = size;
    printer = new QPrinter(QPrinter::ScreenResolution);
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setFullPage(true);
    //printer->setOrientation(QPrinter::Landscape);
    m_tempFile = QDir::tempPath() + QString("temp%1.pdf").arg(i++);
    printer->setOutputFileName(m_tempFile);
}

void PDFEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter p(printer);
    r->render(&p);
    p.end();
}


void PDFEngine::render(const QStringList &qpsScript,
                       const QString &absFilePath)
{
    QPainter pt(printer);
    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(&pt);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    pt.end();
}

bool PDFEngine::drawOnPainter(QPainter *)
{
    return false;
}

void PDFEngine::save(const QString &file)
{
#ifdef USE_ACROBAT
    QString psFile = m_tempFile;
    psFile.replace(".pdf", ".ps");
    QProcess toPs;
    QStringList args1;
    args1 << "-toPostScript"
          << "-level3"
          << "-transQuality"
          << "1";
    args1 << m_tempFile;
    toPs.start("acroread", args1);
    toPs.waitForFinished();

    QProcess convert;
    QStringList args;
    args << psFile;
    args << QString("-resize")
         << QString("%1x%2")
        .arg(m_size.width())
        .arg(m_size.height());
    args << file;

    convert.start("convert", args);
    convert.waitForFinished();
    QFile::remove(psFile);
#else
    QProcess toPng;
    QStringList args1;
    args1 << "-sDEVICE=png16m"
          << QString("-sOutputFile=") + file
          << "-r97x69"
          << "-dBATCH"
          << "-dNOPAUSE";
    args1 << m_tempFile;
    toPng.start("gs", args1);
    toPng.waitForFinished();
#endif

     QString pfile = file;
     pfile.replace(".png", ".pdf");
     QFile::rename(m_tempFile, pfile);
//    QFile::remove(m_tempFile);
}

void PDFEngine::cleanup()
{
    delete printer; printer = 0;
}

#ifdef Q_WS_X11
PSEngine::PSEngine()
{
}


QString PSEngine::name() const
{
    return QLatin1String("PS");
}

void PSEngine::prepare(const QSize &size)
{
    static int i = 1;

    m_size = size;
    printer = new QPrinter(QPrinter::ScreenResolution);
    printer->setOutputFormat(QPrinter::PostScriptFormat);
    printer->setFullPage(true);
    m_tempFile = QDir::tempPath() + QString("temp%1.ps").arg(i++);
    printer->setOutputFileName(m_tempFile);
}

void PSEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter p(printer);
    r->render(&p);
    p.end();
}


void PSEngine::render(const QStringList &qpsScript,
                      const QString &absFilePath)
{
    QPainter pt(printer);
    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(&pt);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    pt.end();
}

bool PSEngine::drawOnPainter(QPainter *)
{
    return false;
}

void PSEngine::save(const QString &file)
{
    QProcess toPng;
    QStringList args1;
    args1 << "-sDEVICE=png16m"
          << QString("-sOutputFile=") + file
          << "-r97x69"
          << "-dBATCH"
          << "-dNOPAUSE";
    args1 << m_tempFile;
    toPng.start("gs", args1);
    toPng.waitForFinished();

    QString pfile = file;
    pfile.replace(".png", ".ps");
    QFile::rename(m_tempFile, pfile);
}

void PSEngine::cleanup()
{
    delete printer; printer = 0;
}
#endif

RSVGEngine::RSVGEngine()
{

}

QString RSVGEngine::name() const
{
    return QLatin1String("RSVG");
}

void RSVGEngine::prepare(const QSize &size)
{
    m_size = size;
}

void RSVGEngine::render(QSvgRenderer *, const QString &fileName)
{
    m_fileName = fileName;
}

void RSVGEngine::render(const QStringList &, const QString &)
{
}

bool RSVGEngine::drawOnPainter(QPainter *)
{
    return false;
}


void RSVGEngine::save(const QString &file)
{
    QProcess rsvg;
    QStringList args;
    args << QString("-w %1").arg(m_size.width());
    args << QString("-h %1").arg(m_size.height());
    args << m_fileName;
    args << file;
    rsvg.start("rsvg", args);
    rsvg.waitForFinished();
}

void QEngine::cleanup()
{
}

#ifdef Q_WS_WIN
WinPrintEngine::WinPrintEngine()
{
}


QString WinPrintEngine::name() const
{
    return QLatin1String("WinPrint");
}

void WinPrintEngine::prepare(const QSize &size)
{
    static int i = 1;

    m_size = size;
    printer = new QPrinter(QPrinter::ScreenResolution);
    printer->setFullPage(true);
	printer->setPrinterName("HP 2500C Series PS3");
    m_tempFile = QDir::tempPath() + QString("temp%1.ps").arg(i++);
    printer->setOutputFileName(m_tempFile);
}

void WinPrintEngine::render(QSvgRenderer *r, const QString &)
{
    QPainter p(printer);
    r->render(&p);
    p.end();
}


void WinPrintEngine::render(const QStringList &qpsScript,
                      const QString &absFilePath)
{
    QPainter pt(printer);
    PaintCommands pcmd(qpsScript, 800, 800);
    pcmd.setPainter(&pt);
    pcmd.setFilePath(absFilePath);
    pcmd.runCommands();
    pt.end();
}

bool WinPrintEngine::drawOnPainter(QPainter *)
{
    return false;
}

void WinPrintEngine::save(const QString &file)
{
    QProcess toPng;
    QStringList args1;
    args1 << "-sDEVICE=png16m"
          << QString("-sOutputFile=") + file
          << "-r97x69"
          << "-dBATCH"
          << "-dNOPAUSE";
    args1 << m_tempFile;
    toPng.start("gswin32", args1);
    toPng.waitForFinished();

    QString pfile = file;
    pfile.replace(".png", ".ps");
    QFile::rename(m_tempFile, pfile);
}

void WinPrintEngine::cleanup()
{
    delete printer; printer = 0;
}

#endif
