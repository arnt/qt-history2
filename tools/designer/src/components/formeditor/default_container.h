#ifndef DEFAULT_CONTAINER_H
#define DEFAULT_CONTAINER_H

#include <container.h>
#include <extension.h>
#include <default_extensionfactory.h>

class QDesignerContainer: public QObject, public IContainer
{
    Q_OBJECT
    Q_INTERFACES(IContainer)
public:
    QDesignerContainer(QWidget *widget, QObject *parent = 0);
    virtual ~QDesignerContainer();

    virtual int count() const;
    virtual QWidget *widget(int index) const;

    virtual int currentIndex() const;
    virtual void setCurrentIndex(int index);

    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    QWidget *m_widget;
};

class QDesignerContainerFactory: public DefaultExtensionFactory
{
    Q_OBJECT
public:
    QDesignerContainerFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // DEFAULT_CONTAINER_H
