#ifndef SHOWER_H
#define SHOWER_H

#include <QWidget>
#include <QImage>
#include <QFileInfo>
#include <QStringList>
#include <QSize>

QT_DECLARE_CLASS(QSvgRenderer)
QT_DECLARE_CLASS(QPaintEvent)
QT_DECLARE_CLASS(QEngine)

class Shower : public QWidget
{
    Q_OBJECT
public:
    Shower(const QString &file,
           const QString &engine);

    QSize sizeHint() const;
protected:
    void paintEvent(QPaintEvent *e);
private:
    QEngine *engine;
    QSvgRenderer *renderer;
    QImage buffer;
    QStringList qpsScript;
    QFileInfo qps;
    QString baseDataDir;
};

#endif
