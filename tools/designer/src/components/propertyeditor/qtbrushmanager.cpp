#include "qtbrushmanager.h"
#include <QPixmap>
#include <QPainter>

class QtBrushManagerPrivate
{
    QtBrushManager *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushManager)
public:
    QMap<QString, QBrush> theBrushMap;
    QString theCurrentBrush;
};


QtBrushManager::QtBrushManager(QObject *parent)
    : QObject(parent)
{
    d_ptr = new QtBrushManagerPrivate;
    d_ptr->q_ptr = this;

}

QtBrushManager::~QtBrushManager()
{
    delete d_ptr;
}

QBrush QtBrushManager::brush(const QString &name) const
{
    if (d_ptr->theBrushMap.contains(name))
        return d_ptr->theBrushMap[name];
    return QBrush();
}

QMap<QString, QBrush> QtBrushManager::brushes() const
{
    return d_ptr->theBrushMap;
}

QString QtBrushManager::currentBrush() const
{
    return d_ptr->theCurrentBrush;
}

QString QtBrushManager::addBrush(const QString &name, const QBrush &brush)
{
    if (name.isNull())
        return QString();

    QString newName = name;
    QString nameBase = newName;
    int i = 0;
    while (d_ptr->theBrushMap.contains(newName)) {
        newName = nameBase + QString::number(++i);
    }
    d_ptr->theBrushMap[newName] = brush;
    emit brushAdded(newName, brush);

    return newName;
}

void QtBrushManager::removeBrush(const QString &name)
{
    if (!d_ptr->theBrushMap.contains(name))
        return;
    if (currentBrush() == name)
        setCurrentBrush(QString());
    emit brushRemoved(name);
    d_ptr->theBrushMap.remove(name);
}

void QtBrushManager::setCurrentBrush(const QString &name)
{
    QBrush newBrush;
    if (!name.isNull()) {
        if (d_ptr->theBrushMap.contains(name))
            newBrush = d_ptr->theBrushMap[name];
        else
            return;
    }
    d_ptr->theCurrentBrush = name;
    emit currentBrushChanged(name, newBrush);
}

QPixmap QtBrushManager::brushPixmap(const QBrush &brush) const
{
    int w = 48;
    int h = 48;

    /*
    int pixSize = 5;
    QPixmap pm(2 * pixSize, 2 * pixSize);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, pixSize, pixSize, Qt::lightGray);
    pmp.fillRect(pixSize, pixSize, pixSize, pixSize, Qt::lightGray);
    pmp.fillRect(0, pixSize, pixSize, pixSize, Qt::darkGray);
    pmp.fillRect(pixSize, 0, pixSize, pixSize, Qt::darkGray);
    */

    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    if (brush.style() == Qt::LinearGradientPattern ||
            brush.style() == Qt::RadialGradientPattern ||
            brush.style() == Qt::ConicalGradientPattern) {
        p.scale(w, h);
        p.fillRect(QRect(0, 0, 1, 1), brush);
    } else {
        p.fillRect(QRect(0, 0, w, h), brush);
    }

    return QPixmap::fromImage(img);
}

