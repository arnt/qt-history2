#ifndef QSHORTCUT_H
#define QSHORTCUT_H

#include "qobject.h"
#include "qwidget.h"
#include "qkeysequence.h"

class QShortcutPrivate;
class Q_GUI_EXPORT QShortcut : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QShortcut);
    Q_PROPERTY(QKeySequence key READ key WRITE setKey)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
public:
    QShortcut(QWidget *parent, const char *name = 0);
    QShortcut(const QKeySequence& key, QWidget *parent,
              const char *member = 0, const char *ambiguousMember = 0,
              const char *name = 0);
    ~QShortcut();

    void setKey(const QKeySequence& key);
    QKeySequence key() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

    void setWhatsThis(const QString &text);
    QString whatsThis() const;

    int id() const;

    inline QWidget *parentWidget() const
    { return static_cast<QWidget *>(QObject::parent()); }

signals:
    void activated();
    void activatedAmbiguously();

protected:
    QShortcut(QShortcutPrivate &d, QWidget *parent);
    void init(QWidget *parent, const char *name);
    bool eventFilter(QObject *o, QEvent *e);
};

#endif // QSHORTCUT_H
