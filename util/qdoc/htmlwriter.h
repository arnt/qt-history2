/*
  htmlwriter.h
*/

#ifndef HTMLWRITER_H
#define HTMLWRITER_H

#include <qmap.h>
#include <qstring.h>

#include <stdio.h>

#include "stringset.h"

class HtmlWriter
{
public:
    static void setStyle( const QString& style ) { styl = style; }
    static void setPostHeader( const QString& html ) { posth = html; }
    static void setAddress( const QString& html ) { addr = html; }
    static const QMap<QString, StringSet>& titleMap() { return tmap; }

    HtmlWriter( const QString& fileName );
    ~HtmlWriter();

    void setTitle( const QString& title ) { t = title; }
    void setHeading( const QString& heading ) { h = heading; }
    void setSubHeading( const QString& subHeading ) { sh = subHeading; }
    void enterFooter();

    void printfMeta( const char *fmt, ... );
    void putsMeta( const char *str );
    void puts( const char *str );

    void printFnord();

    const QString& fileName() const { return fn; }

private:
#if defined(Q_DISABLE_COPY)
    HtmlWriter( const HtmlWriter& );
    HtmlWriter& operator=( const HtmlWriter& );
#endif

    void flushHead();

    FILE *out;
    QString fn;
    QString t;
    QString h;
    QString sh;
    bool headFlushed;
    bool footFlushed;

    static QMap<QString, StringSet> tmap;
    static QString styl;
    static QString posth;
    static QString addr;
};

#endif
