#ifndef LANGUAGEINTERFACEIMPL_H
#define LANGUAGEINTERFACEIMPL_H

#include <languageinterface.h>

class LanguageInterfaceImpl : public LanguageInterface
{
public:
    LanguageInterfaceImpl();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    void functions( const QString &code, QMap<QString, QString> *funcs ) const;
    QString createFunctionStart( const QString &className, const QString &func );
    QStringList definitions() const;
    QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const;
    void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface );

private:
    ulong ref;

};

#endif
