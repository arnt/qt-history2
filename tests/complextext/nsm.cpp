#include "../../src/kernel/qcomplextext_p.h"

#include <qscrollview.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpointarray.h>

QPointArray positionMarks( const QFont &f, const QString &str, int pos )
{
    int len = str.length();
    int nmarks = 0;
    while ( pos + nmarks < len && str[pos+nmarks +1].combiningClass() > 0 )
        nmarks++;

    if ( !nmarks )
        return QPointArray();

    QFontMetrics fm( f );
    QChar baseChar = QComplexText::shapedCharacter( str, pos );
    QRect baseRect = fm.boundingRect( baseChar );

    QPointArray pa( nmarks );
    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    for( i = 0; i < nmarks; i++ ) {
        QChar mark = str[pos+i+1];
        unsigned char cmb = mark.combiningClass();
        // combining marks of different class don't interact. Reset the rectangle.
        if ( cmb != lastCmb ) {
            //qDebug( "resetting rect" );
            attachmentRect = baseRect;
        }

        QPoint p;
        QRect markRect = fm.boundingRect( mark );
        switch( cmb ) {
        case QChar::Combining_DoubleBelow:
                // ### wrong in rtl context!
        case QChar::Combining_BelowLeft:
            p += QPoint( 0, 1 );
        case QChar::Combining_BelowLeftAttached:
            p += attachmentRect.bottomLeft() - markRect.topLeft();
            break;
        case QChar::Combining_Below:
            p += QPoint( 0, 2 );
        case QChar::Combining_BelowAttached:
            p += attachmentRect.bottomLeft() - markRect.topLeft();
            p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
            break;
            case QChar::Combining_BelowRight:
            p += QPoint( 0, 1 );
        case QChar::Combining_BelowRightAttached:
            p += attachmentRect.bottomRight() - markRect.topRight();
            break;
            case QChar::Combining_Left:
            p += QPoint( -1, 0 );
        case QChar::Combining_LeftAttached:
            break;
            case QChar::Combining_Right:
            p += QPoint( 1, 0 );
        case QChar::Combining_RightAttached:
            break;
        case QChar::Combining_DoubleAbove:
            // ### wrong in RTL context!
        case QChar::Combining_AboveLeft:
            p += QPoint( 0, -1 );
        case QChar::Combining_AboveLeftAttached:
            p += attachmentRect.topLeft() - markRect.bottomLeft();
            break;
            case QChar::Combining_Above:
            p += QPoint( 0, -2 );
        case QChar::Combining_AboveAttached:
            p += attachmentRect.topLeft() - markRect.bottomLeft();
            p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
            break;
            case QChar::Combining_AboveRight:
            p += QPoint( 0, -1 );
        case QChar::Combining_AboveRightAttached:
            p += attachmentRect.topRight() - markRect.bottomRight();
            break;

        case QChar::Combining_IotaSubscript:
            default:
                break;
        }
        //qDebug( "char=%x combiningClass = %d offset=%d/%d", mark.unicode(), cmb, p.x(), p.y() );
        markRect.moveBy( p.x(), p.y() );
        attachmentRect = attachmentRect | markRect;
        lastCmb = cmb;
        pa.setPoint( i, p );
    }
    return pa;
}

class MyView : public QScrollView
{
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch )
    {
        p->fillRect( cx, cy, cw, ch, Qt::white );
        QString text = QString::fromUtf8( " Tại sao họ không thể chỉ nói tiếng Việ́t" );
        //QString text = QString::fromUtf8( "ỏ̉̉̉");
	//QString text = QString::fromUtf8( "לְמָה לָא יאםרוּן בְּאַרָמִית?");
        QFont f;
        f.setFamily( "tahoma" );
        f.setPointSize( 24 );
        QFontMetrics fm(f);
        p->setFont( f );
        int height = fm.height();
        int y = 10;
        int x = 10;
        for( int i = 0; i < text.length(); i++ ) {
            if ( !text[i].isMark() )
                y += height + 5;
            QPointArray pa = positionMarks( f, text, i );

            p->setPen( Qt::red );
            QRect br = fm.boundingRect( text.at(i) );
            br.moveBy( x, y );
            p->drawRect( br );
            p->setPen( Qt::black );
            p->drawText( x, y, text, i, 1 );

            for( int j = 0; j < pa.size(); j++ ) {
                int xpos = x + pa[j].x();
                int ypos = y + pa[j].y();

                p->setPen( Qt::blue );
                QRect br = fm.boundingRect( text.at(i+j+1) );
                br.moveBy( xpos, ypos );
                p->drawRect( br );
                p->setPen( Qt::black );
                p->drawText( xpos, ypos, text, i+j+1, 1 );
            }
            i += pa.size();
        }
        resizeContents( 200, y + 100 );
    }
};


int main( int argc, char **argv )
{
    QApplication a(argc, argv);

    MyView *myview = new MyView();
    a.setMainWidget( myview );

    myview->resize( 100, 500 );
    myview->show();

    return a.exec();
}
