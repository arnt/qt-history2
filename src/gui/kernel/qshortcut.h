#ifndef QSHORTCUT_H
#define QSHORTCUT_H

#include "qobject.h"
#include "qwidget.h"
#include "qkeysequence.h"

class QShortcutPrivate;
class Q_GUI_EXPORT QShortcut : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QShortcut)
    Q_PROPERTY(QKeySequence key READ key WRITE setKey)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(Qt::ShortcutContext context READ context WRITE setContext)
public:
    QShortcut(QWidget *parent);
    QShortcut(const QKeySequence& key, QWidget *parent,
              const char *member = 0, const char *ambiguousMember = 0,
              Qt::ShortcutContext context = Qt::ShortcutOnActiveWindow);
    ~QShortcut();

    void setKey(const QKeySequence& key);
    QKeySequence key() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

    void setContext(Qt::ShortcutContext context);
    Qt::ShortcutContext context();

    void setWhatsThis(const QString &text);
    QString whatsThis() const;

    int id() const;

    inline QWidget *parentWidget() const
    { return static_cast<QWidget *>(QObject::parent()); }

signals:
    void activated();
    void activatedAmbiguously();

protected:
    bool event(QEvent *e);
};

#endif // QSHORTCUT_H
