#include <gendocrules.h>

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QString>

#include <iostream>
using namespace std;

/*
    Read text file contents.
*/

QString loadDocFile( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open(QIODevice::ReadOnly) ) {
        fprintf( stderr, "error: Cannot load '%s': %s\n",
                 file.fileName().latin1(),
                 file.errorString().latin1() );
        return QString();
    }
    QTextStream in( &file );
    return in.read();
}

GenDocRules::GenDocRules(QString filename)
:filename(filename)
{

}

/*
    Find SymbolRename duplicates.
    returns true if ren already exists in renameList, false otherwise.
*/
bool GenDocRules::isDefined(SymbolRename ren, QList<SymbolRename> renamesList)
{
    foreach(SymbolRename currentRename, renamesList) {
        if(currentRename.from == ren.from && currentRename.to == ren.to){
            puts("found duplicate rule def in " + filename);
            return true;
        }
    }
    return false;
}

/*
    Parse a .qdocinc file containing symbol rename rules
    in the form \row \o From \o To. Return a list of the renames.
*/
QList<SymbolRename> GenDocRules::getSymbolRenames()
{
    QString fileContents = loadDocFile(filename);
    QStringList list =  fileContents.split(QRegExp("\\s*(\\\\row|\\\\o|\\\n)\\s*"), QString::SkipEmptyParts);

 //   qDebug("symbol renames");
#if 0
    qDebug(fileContents);
    foreach(QString string, list)
    {
             qDebug("|"+string+"|");
    }
#endif
    QList<SymbolRename> symbolRenames;
    int i=0;
    while(i<list.size())
    {
        SymbolRename symbolRename;
        symbolRename.from = list[i].trimmed();
        symbolRename.to = list[++i].trimmed();

        QString str;
        symbolRename.from.toLatin1();
/*
        cout << "renaming " << symbolRename.from.toLatin1().constData()
             << " to " << symbolRename.to.toLatin1().constData() << endl;
*/
        ++i;
        if(!isDefined(symbolRename, symbolRenames))
            symbolRenames.append(symbolRename);

    }
    return symbolRenames;
}

/*
    Parse a .qdocinc file containing symbol remove rules
    in the form \row \o remove. Return a list of removes.
*/

QStringList GenDocRules::getSymbolRemoves()
{
    QString fileContents = loadDocFile(filename);
    QStringList list =  fileContents.split(QRegExp("\\s*(\\\\row|\\\\o|\\\n)\\s*"), QString::SkipEmptyParts);

//    qDebug("symbol removes");
#if 0
    qDebug(fileContents);
    foreach(QString string, list)
    {
             qDebug("|"+string+"|");
    }
#endif
    return list;

}

