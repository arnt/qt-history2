#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <QAbstractButton>

#include <taskmenu.h>
#include <default_extensionfactory.h>

class ButtonTaskMenu: public QObject, public ITaskMenu
{
    Q_OBJECT
    Q_INTERFACES(ITaskMenu)
public:
    ButtonTaskMenu(QAbstractButton *button, QObject *parent = 0);
    virtual ~ButtonTaskMenu();

    virtual QList<QAction*> taskActions() const;

private slots:
    void editText();
    void editIcon();

private:
    QAbstractButton *m_button;
    mutable QList<QAction*> m_taskActions;
};

class ButtonTaskMenuFactory: public DefaultExtensionFactory
{
    Q_OBJECT
public:
    ButtonTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // BUTTON_TASKMENU_H
