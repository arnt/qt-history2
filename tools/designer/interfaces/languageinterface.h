#ifndef LANGUAGEINTERFACE_H
#define LANGUAGEINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>
#include <qmap.h>

// {f208499a-6f69-4883-9219-6e936e55a330}
#ifndef IID_LanguageInterface
#define IID_LanguageInterface QUuid( 0xf208499a, 0x6f69, 0x4883, 0x92, 0x19, 0x6e, 0x93, 0x6e, 0x55, 0xa3, 0x30 )
#endif

struct LanguageInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
    virtual void functions( const QString &code, QMap<QString, QString> *funcs ) const = 0;
    virtual QString createFunctionStart( const QString &className, const QString &func ) = 0;

};

#endif
