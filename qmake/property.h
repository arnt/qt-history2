#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <qstring.h>

class QSettings;

class QMakeProperty 
{
    QSettings *sett;
    QString keyBase(bool =TRUE) const;
    bool initSettings();
    QString value(QString, bool just_check); 
public:
    QMakeProperty();
    ~QMakeProperty();

    bool hasValue(QString);
    QString value(QString v) { return value(v, FALSE); }
    void setValue(QString, const QString &);

    bool exec();
};

#endif /* __PROPERTY_H__ */
