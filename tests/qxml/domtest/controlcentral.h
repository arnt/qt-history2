#include <qvbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qfiledialog.h>

#ifndef CONTROLCENTRAL_H
#define CONTROLCENTRAL_H

class ControlCentral : public QVBox
{
    Q_OBJECT

public:
    ControlCentral();
    ~ControlCentral();

    QSize sizeHint() const;

signals:

public slots:
    void show( QStringList* files=0 );
    void showSource();
    void showToString();
    void saveToFile();
    void showTree();
    void showTree( QListViewItem* );

private:
    void parse( const QString& filename );

    QListView* lview;
    QPushButton* quit;
};

#endif // CONTROLCENTRAL_H
