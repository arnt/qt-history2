#include <qmultilined.h>

class QMultiLineEdit;
class FontSelect;
class QLCDNumber;
class MDialog : public QWidget
{
    Q_OBJECT;
public:
    MDialog();
public slots:
    void setFont( const QFont & );
    void newFont();
    void urk();
    void countUp();
protected:
    bool eventFilter( QEvent * );
private:
    QMultiLineEdit *m;
    FontSelect *fs;
    QLCDNumber *lcd;
    bool fsUp;
};


//////////////////

#include <qdialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qscrollbar.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <stdlib.h>


class FontSelect : public QDialog
{
    Q_OBJECT
public:
    FontSelect( QWidget *parent=0, const char *name=0 );
private slots:
    void	 newStrikeOut();
    void	 doFont();
    void	 newUnderline();
    void	 newItalic();
    void	 newWeight( int id );
    void	 newFamily();
    void	 newSize( int );
    void	 slidingSize( int );
    void	 doApply();
    void	 fileActivated( int );
    void	 familyActivated( int );
    void	 pSizeActivated( int );
signals:
    void	 newFont( const QFont & );
protected:
    void	 updateMetrics();
private:
    QLabel	 *familyL;
    QLabel	 *sizeL;
    QLineEdit	 *family;
    QLineEdit	 *sizeE;
    QCheckBox	 *italic;
    QCheckBox	 *underline;
    QCheckBox	 *strikeOut;
    QPushButton	 *apply;
    QPushButton	 *stat;
    QScrollBar	 *sizeS;
    QRadioButton *rb[5];
    QButtonGroup *weight;
    QGroupBox	 *mGroup;
    QFont	  f;
    QMenuBar	 *menu;
    QLabel	 *metrics[4][2];
    QWidget	 *fontInternal;
};
