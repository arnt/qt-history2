#ifndef ICONVIEW_H
#define ICONVIEW_H

#include <qiconview.h>
#include <qstring.h>
#include <qvaluelist.h>

class IconViewItem : public QIconViewItem
{
public:
    IconViewItem( QIconView * parent, const QString & text, const QPixmap & icon, const QString& tag )
        : QIconViewItem( parent, text, icon ), _tag( tag ) {}
    virtual ~IconViewItem() {}

    QString tag() { return _tag; }

private:
    QString _tag;
};

class IconView : public QIconView
{
    Q_OBJECT

public:
    IconView( QWidget* parent = 0, const char* name = 0 );
    ~IconView();

    QDragObject *dragObject();

public slots:
    void slotNewItem( QDropEvent *t, const QValueList<QIconDragItem>& );
};

#endif // ICONVIEW_H
