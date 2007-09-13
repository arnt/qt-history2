#ifndef QENGINES_H
#define QENGINES_H

#if defined(BUILD_OPENGL)
#include <QGLPixelBuffer>
#endif
#include <QPrinter>
#include <QPixmap>
#include <QImage>
#include <QMap>
#include <QList>

QT_DECLARE_CLASS(QSvgRenderer)
QT_DECLARE_CLASS(QGLWidget)

class QEngine
{
public:
    virtual ~QEngine();
    virtual QString name() const =0;
    virtual void prepare(const QSize &size) =0;
    virtual void render(QSvgRenderer *r, const QString &)    =0;
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath)    =0;
    virtual bool drawOnPainter(QPainter *p) =0;
    virtual void save(const QString &file)  =0;
    virtual void cleanup();
};

class QtEngines
{
public:
    static QtEngines *self();
    QtEngines();

    QList<QEngine*> engines() const;
    QList<QEngine*> foreignEngines() const;

    QEngine *defaultEngine() const;
private:
    void init();
private:
    QList<QEngine*> m_engines;
    QList<QEngine*> m_foreignEngines;
    QEngine        *m_defaultEngine;
};

class RasterEngine : public QEngine
{
public:
    RasterEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
private:
    QImage image;
};

class NativeEngine : public QEngine
{
public:
    NativeEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
private:
    QPixmap pixmap;
};


#if defined(BUILD_OPENGL)
class GLEngine : public QEngine
{
public:
    GLEngine();
    virtual QString name() const;
    virtual void prepare(const QSize &_size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
    virtual void cleanup();
private:
    QGLPixelBuffer *pbuffer;
    QGLWidget *widget;
    bool usePixelBuffers;
    QSize size;
};
#endif

class PDFEngine : public QEngine
{
public:
    PDFEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
    virtual void cleanup();
private:
    QPrinter *printer;
    QSize     m_size;
    QString   m_tempFile;
};

#ifdef Q_WS_X11
class PSEngine : public QEngine
{
public:
    PSEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
    virtual void cleanup();
private:
    QPrinter *printer;
    QSize     m_size;
    QString   m_tempFile;
};
#endif

#ifdef Q_WS_WIN
class WinPrintEngine : public QEngine
{
public:
    WinPrintEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
    virtual void cleanup();
private:
    QPrinter *printer;
    QSize     m_size;
    QString   m_tempFile;
};
#endif


class RSVGEngine : public QEngine
{
public:
    RSVGEngine();

    virtual QString name() const;
    virtual void prepare(const QSize &size);
    virtual void render(QSvgRenderer *r, const QString &);
    virtual void render(const QStringList &qpsScript,
                        const QString &absFilePath);
    virtual bool drawOnPainter(QPainter *p);
    virtual void save(const QString &file);
private:
    QString m_fileName;
    QSize   m_size;
};

#endif
