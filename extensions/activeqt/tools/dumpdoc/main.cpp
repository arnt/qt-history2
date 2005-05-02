/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qaxobject.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qt_windows.h>

int main(int argc, char **argv)
{
    CoInitialize(0);

    enum State {
        Default = 0,
            OutOption
    } state;
    state = Default;
    
    QByteArray outname;
    QByteArray object;
    
    for (int a = 1; a < argc; ++a) {
        QByteArray arg(argv[a]);
        const char first = arg[0];
        switch(state) {
        case Default:
            if (first == '-' || first == '/') {
                arg = arg.mid(1);
                arg.toLower();
                if (arg == "o")
                    state = OutOption;
                else if (arg == "v") {
                    qWarning("dumpdoc: Version 1.0");
                    return 0;
                } else if (arg == "h") {
                    qWarning("dumpdoc Usage:\n\tdumpdoc object [-o <file>]"
                        "              \n\tobject   : object[/subobject]*"
                        "              \n\tsubobject: property\n"
                        "      \nexample:\n\tdumpdoc Outlook.Application/Session/CurrentUser -o outlook.html");
                    return 0;
                }
            } else {
                object = arg;
            }
            break;
        case OutOption:
            outname = arg;
            state = Default;
            break;
            
        default:
            break;
        }
    }
    
    if (object.isEmpty()) {
        qWarning("dumpdoc: No object name provided.\n"
            "         Use -h for help.");
        return -1;
    }
    QFile outfile;
    if (!outname.isEmpty()) {
        outfile.setFileName(outname);
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpdoc: Could not open output file '%s'", outname.data());
        }
    } else {
        outfile.open(stdout, QIODevice::WriteOnly);
    }
    QTextStream out(&outfile);
    
    QByteArray subobject = object;
    int index = subobject.indexOf('/');
    if (index != -1)
        subobject = subobject.left(index);
    
    QAxObject topobject(subobject);
    
    if (topobject.isNull()) {
        qWarning("dumpdoc: Could not instantiate COM object '%s'", subobject.data());
        return -2;
    }
    
    QAxObject *axobject = &topobject;
    while (index != -1 && axobject) {
        index++;
        subobject = object.mid(index);
        if (object.indexOf('/', index) != -1) {
            int oldindex = index;
            index = object.indexOf('/', index);
            subobject = object.mid(oldindex, index-oldindex);	    
        } else {
            index = -1;
        }
        
        axobject = axobject->querySubObject(subobject);
    }
    if (!axobject || axobject->isNull()) {
        qWarning("dumpdoc: Subobject '%s' does not exist in '%s'", subobject.data(), object.data());
        return -3;
    }
    
    QString docu = axobject->generateDocumentation();
    out << docu;
    
    return 0;
}
