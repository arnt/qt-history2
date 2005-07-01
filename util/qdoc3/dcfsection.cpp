#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <QtXml>

#include "dcfsection.h"
#include "htmlgenerator.h"

void appendDcfSubSection( DcfSection *dcfSect, const DcfSection& sub )
{
    dcfSect->subsections.append( sub );
}

void appendDcfSubSections( DcfSection *dcfSect, const QList<DcfSection>& subs )
{
    dcfSect->subsections += subs;
}

void generateDcfSubSections( QString indent, QTextStream& out, const DcfSection& sect )
{
    QList<DcfSection>::const_iterator ss = sect.subsections.constBegin();
    while ( ss != sect.subsections.constEnd() ) {
	out << indent << "<section ref=\"" << HtmlGenerator::protect((*ss).ref)
            << "\" title=\"" << HtmlGenerator::protect((*ss).title) << "\"";
        if (!(*ss).bases.isNull())
            out << " bases=\"" << HtmlGenerator::protect((*ss).bases) << "\"";

	if ((*ss).keywords.isEmpty() && (*ss).subsections.isEmpty()) {
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
    out.flush();
}

void generateDcfSections( const DcfSection& rootSect, const QString& fileName,
			  const QString& /* category */ )
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return ;

    QTextStream out(&file);

    QString icon = QFileInfo(fileName).baseName() + ".png";

    out << "<!DOCTYPE DCF>\n";
    out << "<DCF ref=\"" << HtmlGenerator::protect(rootSect.ref);
    if (icon != "qmake.png")
	out << "\" icon=\"" << HtmlGenerator::protect(icon);
    out << "\" imagedir=\"../../gif\" title=\"" << HtmlGenerator::protect(rootSect.title) +
	      "\">\n";

    generateDcfSubSections( "", out, rootSect );

    out << "</DCF>\n";
    out.flush();
}

DcfSection readDcfFile(const QString &path)
{
    DcfSection section;

    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QDomDocument document;
        document.setContent(&file);
        file.close();

        section = readDcfSection(document.documentElement());
    }
    return section;
}

DcfSection readDcfSection(const QDomElement &element)
{
    DcfSection section;
    section.title = element.attribute("title");
    section.ref = element.attribute("ref");

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        if (child.nodeName() == "keyword") {
            QString text = readDcfText(child);
            if (!text.isEmpty())
                section.keywords.append(qMakePair(text, child.attribute("ref")));
        } else if (child.nodeName() == "section")
            section.subsections.append(readDcfSection(child));

        child = child.nextSiblingElement();
    }
    return section;
}

QString readDcfText(const QDomElement &element)
{
    QString text;
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        if (child.isText())
            text += child.toText().nodeValue();
        child = child.nextSibling();
    }
    return text;
}
