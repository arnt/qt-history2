#ifndef ABSTRACTWIDGETFACTORY_H
#define ABSTRACTWIDGETFACTORY_H

#include "sdk_global.h"
#include <QObject>

class AbstractFormEditor;
class QWidget;
class QLayout;

class QT_SDK_EXPORT AbstractWidgetFactory: public QObject
{
    Q_OBJECT
public:
    AbstractWidgetFactory(QObject *parent = 0);
    virtual ~AbstractWidgetFactory();

    virtual AbstractFormEditor *core() const = 0;

    virtual QWidget* containerOfWidget(QWidget *w) const = 0;
    virtual QWidget* widgetOfContainer(QWidget *w) const = 0;

    virtual QWidget *createWidget(const QString &name, QWidget *parentWidget = 0) const = 0;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const = 0;

    virtual void initialize(QObject *object) const = 0;
};

#endif // ABSTRACTWIDGETFACTORY_H
