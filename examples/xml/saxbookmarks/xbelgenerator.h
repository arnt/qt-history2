#ifndef XBELGENERATOR_H
#define XBELGENERATOR_H

#include <QTextStream>

class QTreeWidget;
class QTreeWidgetItem;

class XbelGenerator
{
public:
    XbelGenerator(QTreeWidget *treeWidget);

    bool write(QIODevice *device);

private:
    static QString indent(int indentLevel);
    static QString escapedText(const QString &str);
    static QString escapedAttribute(const QString &str);
    void generateItem(QTreeWidgetItem *item, int depth);

    QTreeWidget *treeWidget;
    QTextStream out;
};

#endif
