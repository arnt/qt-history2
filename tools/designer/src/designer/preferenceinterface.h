// Preference interface for Qt Designer

#ifndef PREFERENCEINTERFACE_H
#define PREFERENCEINTERFACE_H

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtGui/QIcon>

class QWidget;

class PreferenceInterface : public QObject
{
    Q_OBJECT
public:
    PreferenceInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~PreferenceInterface() {};
    virtual QWidget *createPreferenceWidget(QWidget *parent = 0) = 0;
    virtual bool settingsChanged() const = 0;
    virtual QString preferenceName() const = 0;
    virtual QIcon preferenceIcon() const = 0;
    virtual bool saveSettings() = 0;
    virtual bool readSettings() = 0;

signals:
    void changed();
};

#endif
