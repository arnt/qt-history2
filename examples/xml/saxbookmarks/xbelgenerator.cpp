#include <QtGui>

#include "xbelgenerator.h"

XbelGenerator::XbelGenerator(QTreeWidget *treeWidget)
    : treeWidget(treeWidget)
{
}

bool XbelGenerator::write(QIODevice *device)
{
    out.setDevice(device);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<!DOCTYPE xbel>\n"
        << "<xbel version=\"1.0\">\n";

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        generateItem(treeWidget->topLevelItem(i), 1);

    out << "</xbel>\n";
    return true;
}

QString XbelGenerator::indent(int depth)
{
    const int IndentSize = 4;
    return QString(IndentSize * depth, ' ');
}

QString XbelGenerator::escapedText(const QString &str)
{
    QString result = str;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    return result;
}

QString XbelGenerator::escapedAttribute(const QString &str)
{
    QString result = escapedText(str);
    result.replace("\"", "&quot;");
    result.prepend("\"");
    result.append("\"");
    return result;
}

void XbelGenerator::generateItem(QTreeWidgetItem *item, int depth)
{
    QString tagName = item->data(0, QAbstractItemModel::UserRole).toString();
    if (tagName == "folder") {
        bool folded = !treeWidget->isItemExpanded(item);
        out << indent(depth) << "<folder folded=\"" << (folded ? "yes" : "no")
                             << "\">\n"
            << indent(depth + 1) << "<title>" << escapedText(item->text(0))
                                 << "</title>\n";

        for (int i = 0; i < item->childCount(); ++i)
            generateItem(item->child(i), depth + 1);

        out << indent(depth) << "</folder>\n";
    } else if (tagName == "bookmark") {
        out << indent(depth) << "<bookmark";
        if (!item->text(1).isEmpty())
            out << " href=" << escapedAttribute(item->text(1));
        out << ">\n"
            << indent(depth + 1) << "<title>" << escapedText(item->text(0))
                                 << "</title>\n"
            << indent(depth) << "</bookmark>\n";
    } else if (tagName == "separator") {
        out << indent(depth) << "<separator/>\n";
    }
}
