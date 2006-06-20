#ifndef PARSER_H
#define PARSER_H

#include <qfile.h>
#include <qstring.h>
#include <qxml.h>

class ContentHandler;

class Parser : public QXmlSimpleReader
{
    public:
	Parser();
	~Parser();

	bool parseFile(QFile *file);
	QString result() const;
        QString errorMsg() const;

    private:
	ContentHandler *handler;
};

#endif
