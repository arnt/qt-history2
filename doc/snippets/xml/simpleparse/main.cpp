#include <qfile.h>
#include <qxml.h>
#include "handler.h"

#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    QFile *file = new QFile(argv[1]);

    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(file);

    Handler *handler = new Handler;
    xmlReader.setContentHandler(handler);
    xmlReader.setErrorHandler(handler);

    bool ok = xmlReader.parse(source);

    if (!ok)
        std::cout << "Parsing failed." << std::endl;
    else {
        QStringList names = handler->names();
        QValueList<int> indentations = handler->indentations();

        int items = names.count();

        for (int i = 0; i < items; ++i) {
            for (int j = 0; j < indentations[i]; ++j)
                std::cout << " ";
            std::cout << names[i] << std::endl;
        }
    }

    return 0;
}
