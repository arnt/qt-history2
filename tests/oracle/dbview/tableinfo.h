/****************************************************************************
** Form interface generated from reading ui file 'tableinfo.ui'
**
** Created: la 8. des 19:32:25 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <qvariant.h>
#include <qtabwidget.h>
#include <qmap.h>
#include "dbconnection.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTable;
class DistributionWidget;

class TableInfo : public QTabWidget
{ 
    Q_OBJECT

public:
    TableInfo( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~TableInfo();

private:
    QWidget* columnsTab;
    QWidget* indexTab;
    QWidget* constraintsTab;
    QWidget* statsTab;
    QWidget* dataTab;
    QWidget* storageTab;
    QLineEdit* tableTableSpace;
    QTable* columnsTable;
    QTable* indexTable;
    QTable* constraintsTable;
    QTable* dataTable;
    QGroupBox* dataStatsBox;
    QLineEdit* dataSize;
    QLineEdit* numDataExtents;
    QLineEdit* numDataBlocks;
    QLineEdit* numRows;
    QGroupBox* indexStatsBox;
    QLineEdit* indexSize;
    QLineEdit* numIndexes;
    QLineEdit* numIndexExtents;
    QLineEdit* numIndexBlocks;
    QGroupBox* totalStatsBox;
    QLineEdit* totalSize;
    QLineEdit* numTotalExtents;
    QLineEdit* numTotalBlocks;
    QGroupBox* dataDistBox;
    QGroupBox* indexDistBox;
    DistributionWidget* dataDist;
    DistributionWidget* indexDist;

    void buildColumnList();
public slots:
    void populate( ObjectInfo info );
    virtual void showPage( QWidget* );
    void clear();
private:
    QStringList columns;
    ObjectInfo info;
    QMap< QWidget*, bool > populated;
};

#endif // TABLEINFO_H
