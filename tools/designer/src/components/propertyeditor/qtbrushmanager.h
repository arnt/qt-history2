#ifndef QTBRUSHMANAGER_H
#define QTBRUSHMANAGER_H

#include <QObject>
#include <QMap>
#include <QBrush>

class QtBrushManager : public QObject
{
    Q_OBJECT
public:
    QtBrushManager(QObject *parent = 0);
    ~QtBrushManager();

    QBrush brush(const QString &name) const;
    QMap<QString, QBrush> brushes() const;
    QString currentBrush() const;

    QString addBrush(const QString &name, const QBrush &brush);
    void removeBrush(const QString &name);
    void setCurrentBrush(const QString &name);

    QPixmap brushPixmap(const QBrush &brush) const;
signals:
    void brushAdded(const QString &name, const QBrush &brush);
    void brushRemoved(const QString &name);
    void currentBrushChanged(const QString &name, const QBrush &brush);

private:
    class QtBrushManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushManager)
    Q_DISABLE_COPY(QtBrushManager)
};

#endif
