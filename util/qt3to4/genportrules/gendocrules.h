#ifndef GENDOCRULES_H
#define GENDOCRULES_H

#include <QString>
#include <QList>

#include "genportrules.h"

class GenDocRules
{
    public:
    GenDocRules(QString filename);
    QList<SymbolRename> getSymbolRenames();
    QStringList         getSymbolRemoves();
private:
    bool isDefined(SymbolRename ren, QList<SymbolRename> renamesList);
    QString filename;
};

#endif
