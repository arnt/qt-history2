#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <qstring.h>

class QSettings;

class QMakeProperty 
{
    QSettings *sett;
    QString keyBase() const;
    bool initSettings();
public:
    QMakeProperty();
    ~QMakeProperty();

    bool hasValue(const QString &);
    QString value(const QString &); 
    void setValue(const QString &, const QString &);

    bool exec();
};

#endif /* __PROPERTY_H__ */
