#include <qdialog.h>
#include <qdom.h>
#include <qdict.h>
#include <qlineedit.h>

#ifndef QNODEEDIT_H
#define QNODEEDIT_H

class NodeEdit : public QDialog
{
    Q_OBJECT

public:
    NodeEdit( QDomNode _node );

private slots:
    void ok();
    void cancel();

private:
    QDomNode node;
    QDict<QLineEdit> edits;
};

#endif
