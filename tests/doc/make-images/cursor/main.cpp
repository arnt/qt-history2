/*
    Use this to create: cursors.png
*/
#include <qapplication.h>
#include <qlistbox.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qstringlist.h>
#include <qhbox.h>
#include "../../../../tools/designer/pics/arrow.xbm"
#include "../../../../tools/designer/pics/uparrow.xbm"
#include "../../../../tools/designer/pics/cross.xbm"
#include "../../../../tools/designer/pics/wait.xbm"
#include "../../../../tools/designer/pics/ibeam.xbm"
#include "../../../../tools/designer/pics/sizeh.xbm"
#include "../../../../tools/designer/pics/sizev.xbm"
#include "../../../../tools/designer/pics/sizeb.xbm"
#include "../../../../tools/designer/pics/sizef.xbm"
#include "../../../../tools/designer/pics/sizeall.xbm"
#include "../../../../tools/designer/pics/vsplit.xbm"
#include "../../../../tools/designer/pics/hsplit.xbm"
#include "../../../../tools/designer/pics/hand.xbm"
#include "../../../../tools/designer/pics/no.xbm"
#include "whatsthis.xpm"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QBitmap cur;
    QHBox *box = new QHBox( (QWidget*)0 );
    QListBox *lbLeft = new QListBox( box );
    QListBox *lbRight = new QListBox( box );

    cur = QBitmap(arrow_width, arrow_height, arrow_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "ArrowCursor" );

    cur = QBitmap(uparrow_width, uparrow_height, uparrow_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "UpArrowCursor" );

    cur = QBitmap(cross_width, cross_height, cross_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "CrossCursor" );

    cur = QBitmap(wait_width, wait_height, wait_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "WaitCursor" );

    cur = QBitmap(ibeam_width, ibeam_height, ibeam_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "IbeamCursor" );

    cur = QBitmap(hand_width, hand_height, hand_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "PointingHandCursor" );

    cur = QBitmap(no_width, no_height, no_bits, TRUE);
    cur.setMask( cur );
    lbLeft->insertItem( cur, "ForbiddenCursor" );

    lbLeft->insertItem( QPixmap( "whatsthis.xpm" ), "  WhatsThisCursor" );

    cur = QBitmap(sizev_width, sizev_height, sizev_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SizeVerCursor" );

    cur = QBitmap(sizeh_width, sizeh_height, sizeh_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SizeHorCursor" );

    cur = QBitmap(sizef_width, sizef_height, sizef_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SizeFDiagCursor" );

    cur = QBitmap(sizeb_width, sizeb_height, sizeb_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SizeBDiagCursor" );

    cur = QBitmap(sizeall_width, sizeall_height, sizeall_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SizeAllCursor" );

    cur = QBitmap(vsplit_width, vsplit_height, vsplit_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SplitVCursor" );

    cur = QBitmap(hsplit_width, hsplit_height, hsplit_bits, TRUE);
    cur.setMask( cur );
    lbRight->insertItem( cur, "SplitHCursor" );

    cur = QBitmap( 25, 25, 1 );
    cur.setMask( cur );
    lbRight->insertItem( cur, "BlankCursor" );

    app.setMainWidget( box );
    box->setCaption( "Cursors" );
    box->resize( 350, 205 );
    box->show();

    return app.exec();
}
