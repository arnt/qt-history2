#include <qfile.h>
#include <qtextstream.h>

#include "dcfsection.h"
#include "htmlgenerator.h"

void appendDcfSubSection( DcfSection *dcfSect, const DcfSection& sub )
{
    if ( dcfSect->subsections == 0 )
	dcfSect->subsections = new QList<DcfSection>;
    dcfSect->subsections->append( sub );
}

void appendDcfSubSections( DcfSection *dcfSect,
			   const QList<DcfSection>& subs )
{
    if ( dcfSect->subsections == 0 )
	dcfSect->subsections = new QList<DcfSection>;
    *dcfSect->subsections += subs;
}

void generateDcfSubSections( QString indent, QTextStream& out, const DcfSection& sect )
{
    if (!sect.subsections)
        return;

    QList<DcfSection>::ConstIterator ss = sect.subsections->begin();
    while ( ss != sect.subsections->end() ) {
	out << indent << "<section ref=\"" << HtmlGenerator::protect((*ss).ref)
            << "\" title=\"" << HtmlGenerator::protect((*ss).title) << "\"";
	if ( (*ss).keywords.isEmpty() && (*ss).subsections == 0 ) {
	    out << "/>\n";
	} else {
	    out << ">\n";
	    QString indentIndent = indent + "    ";
	    QList<QPair<QString, QString> >::const_iterator k = (*ss).keywords.constBegin();
	    while ( k != (*ss).keywords.constEnd() ) {
		out << indentIndent << "<keyword ref=\"" << (*k).second << "\">"
                    << HtmlGenerator::protect((*k).first) << "</keyword>\n";
		++k;
	    }

            generateDcfSubSections( indentIndent, out, *ss );
	    out << indent << "</section>\n";
	}
	++ss;
    }
}

void generateDcfSections( const DcfSection& rootSect, const QString& fileName,
			  const QString& /* category */ )
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Translate))
        return ;

    QTextStream out(&file);

    QString icon = fileName;
    icon.replace( ".dcf", ".png" );

    out << "<!DOCTYPE DCF>\n";
    out << "<DCF ref=\"" << HtmlGenerator::protect(rootSect.ref);
    if (icon != "qmake.png")
	out << "\" icon=\"" << HtmlGenerator::protect(icon);
    out << "\" imagedir=\"../../gif\" title=\"" << HtmlGenerator::protect(rootSect.title) +
	      "\">\n";

    generateDcfSubSections( "", out, rootSect );

    out << "</DCF>\n";
}
