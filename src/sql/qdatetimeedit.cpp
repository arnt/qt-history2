/****************************************************************************
**
** Implementation of date and time edit classes
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

#include "qtoolbutton.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qframe.h"
#include "qlabel.h"
#include "qapplication.h"

class QNumEditPrivate : public QLineEdit
{
public:
    QNumEditPrivate( QWidget * parent, const char * name = 0 )
        : QLineEdit( parent, name )
    {
        setFrame( FALSE );
        setAlignment( AlignRight );
    }

    void setRange( int min, int max )
    {
        QIntValidator * v = new QIntValidator( this );
        v->setRange( min, max );
        setValidator( v );
    }
};

class QDateTimeEditLabelPrivate : public QLabel
{
public:
    QDateTimeEditLabelPrivate( QWidget * parent = 0, const char * name = 0,
                        WFlags f = 0 )
        : QLabel( parent, name, f){}

protected:
    void drawContents( QPainter * p )
    {
        p->fillRect( contentsRect(), colorGroup().background() );
        QLabel::drawContents( p );
    }
};

/*!

  Base class for the QTimeEdit and QDateEdit widgets.
 */
QDateTimeEditBase::QDateTimeEditBase( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
}

/*!

  \internal Initialization.
*/
void QDateTimeEditBase::init()
{
    if( style() == WindowsStyle )
        setFrameStyle( WinPanel | Sunken );
    else
        setFrameStyle( Panel | Sunken );
    setLineWidth( 2 );

    QPalette p = palette();
    p.setColor( QPalette::Active, QColorGroup::Background,
                palette().active().color( QColorGroup::Base ) );
    p.setColor( QPalette::Inactive, QColorGroup::Background,
                palette().inactive().color( QColorGroup::Base ) );
    setPalette( p );

    ed[0] = new QNumEditPrivate( this, "Ed_1" );
    ed[1] = new QNumEditPrivate( this, "Ed_2" );
    ed[2] = new QNumEditPrivate( this, "Ed_3" );

    ed[0]->setMaxLength( 2 );
    ed[1]->setMaxLength( 2 );
    ed[2]->setMaxLength( 2 );

    //### necessary?
//     connect( ed[0], SIGNAL( textChanged( const QString & ) ),
//           this, SIGNAL( valueChanged() ) );
//     connect( ed[1], SIGNAL( textChanged( const QString & ) ),
//           this, SIGNAL( valueChanged() ) );
//     connect( ed[2], SIGNAL( textChanged( const QString & ) ),
//           this, SIGNAL( valueChanged() ) );

    sep[0] = new QDateTimeEditLabelPrivate( this );
    sep[1] = new QDateTimeEditLabelPrivate( this );

    up   = new QPushButton( this );
    up->setFocusPolicy( QWidget::NoFocus );
    up->setAutoDefault( FALSE );
    up->setAutoRepeat( TRUE );

    down = new QPushButton( this );
    down->setFocusPolicy( QWidget::NoFocus );
    down->setAutoDefault( FALSE );
    down->setAutoRepeat( TRUE );

    connect( up, SIGNAL( clicked() ), SLOT( stepUp() ) );
    connect( down, SIGNAL( clicked() ), SLOT( stepDown() ) );

    ed[0]->installEventFilter( this );
    ed[1]->installEventFilter( this );
    ed[2]->installEventFilter( this );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    setFocusProxy( ed[0] );
    setFocusPolicy( StrongFocus );
}

QSize QDateTimeEditBase::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();
    int h = fm.height();
    if ( h < 12 )       // ensure enough space for the button pixmaps
        h = 12;
    int w = 35; // minimum width for the value
    int wx = fm.width( ' ' )*2;
    QString s;
    s = ed[0]->text() + sep[0]->text() +ed[1]->text() + sep[1]->text() +
        ed[2]->text();
    w = QMAX( w, fm.width( s ) + wx);
    QSize r( h // buttons AND frame both sides - see resizeevent()
             + 6 // right/left margins
             + w, // widest value
             frameWidth() * 2 // top/bottom frame
             + 4 // top/bottom margins
             + h // font height
             );
    return r.expandedTo( QApplication::globalStrut() );
}

QSize QDateTimeEditBase::minimumSizeHint() const
{
    int w = 35 // minimum for value
            + 6; // arrows
    int h = 12; // arrow pixmaps
    return QSize( w, h );
}

/*!
  \reimp
*/
void QDateTimeEditBase::drawContents( QPainter * p )
{
    p->fillRect( contentsRect(), colorGroup().background() );
}

/*!

  Draws the arrow buttons.
 */
void QDateTimeEditBase::updateArrows()
{
    QString key( QString::fromLatin1( "$qt$qdatetimeedit$" ) );
    key += QString::fromLatin1( "^v" );
    key += QString::number( down->height() );
    QString upKey = key + QString::fromLatin1( "$up" );
    QString dnKey = key + QString::fromLatin1( "$down" );
    QBitmap upBm;
    QBitmap dnBm;

    bool found = QPixmapCache::find( dnKey, dnBm )
                 && QPixmapCache::find( upKey, upBm );

    if ( !found ) {
        QPainter p;
        int w = down->width()-4;
        if ( w < 3 )
            return;
        else if ( !(w & 1) )
            w--;
        w -= ( w / 7 ) * 2;     // Empty border
        int h = w/2 + 2;        // Must have empty row at foot of arrow
        dnBm.resize( w, h );
        p.begin( &dnBm );
        p.eraseRect( 0, 0, w, h );
        QPointArray a;
        a.setPoints( 3,  0, 1,  w-1, 1,  h-2, h-1 );
        p.setBrush( color1 );
        p.drawPolygon( a );
        p.end();
#ifndef QT_NO_TRANSFORMATIONS
        QWMatrix wm;
        wm.scale( 1, -1 );
        upBm = dnBm.xForm( wm );
#else
        upBm.resize( w, h );
        p.begin( &upBm );
        p.eraseRect( 0, 0, w, h );
        a.setPoints( 3,  0, h-2,  w-1, h-2,  h-2, 0 );
        p.setBrush( color1 );
        p.drawPolygon( a );
        p.end();
#endif
        QPixmapCache::insert( dnKey, dnBm );
        QPixmapCache::insert( upKey, upBm );
    }
    down->setPixmap( dnBm );
    up->setPixmap( upBm );
}

/*!

  Increase the current value one step. This slot is called when the
  up button is clicked.
 */
void QDateTimeEditBase::stepUp()
{
    int i = 0;
    int focus = 0;

    for( i = 0; i < 3; i++)
        if( ed[i]->hasFocus() )
            focus = i;

    QNumEditPrivate * e = ed[focus];
    QIntValidator * v = (QIntValidator *) e->validator();

    if( !v ) return;
    int n = e->text().toInt();
    n++;

    if( n > v->top() )
        n = v->top();
    else if( n < v->bottom() )
        n = v->bottom();

    e->setText( QString::number( n ) );
}

/*!

  Decrease the current value one step. This slot is called when the
  down button is clicked.
 */
void QDateTimeEditBase::stepDown()
{
    int i = 0;
    int focus = 0;

    for( i = 0; i < 3; i++)
        if( ed[i]->hasFocus() )
            focus = i;

    QNumEditPrivate * e = ed[focus];
    QIntValidator * v = (QIntValidator *) e->validator();

    if( !v ) return;
    int n = e->text().toInt();
    n--;

    if( n > v->top() )
        n = v->top();
    else if( n < v->bottom() )
        n = v->bottom();

    e->setText( QString::number( n ) );
}

/*!

  \reimp
 */
bool QDateTimeEditBase::eventFilter( QObject * o, QEvent * e )
{
    if( e->type() == QEvent::KeyPress ){
        QKeyEvent * k = (QKeyEvent *) e;
        // This hack is needed to handle TAB focusing properly
        if( (k->key() == Key_Tab) ) {
            if( o == ed[2] ){
                qApp->sendEvent( this, e );
                return TRUE;
            }
        }
        if( k->key() == Key_BackTab ){
            if( o == ed[0] ){
                qApp->sendEvent( this, e );
                return TRUE;
            }
        }
        if( (k->key() == Key_Up) ){
            stepUp();
            return TRUE;
        }
        if( (k->key() == Key_Down) ){
            stepDown();
            return TRUE;
        }
    }

    if( e->type() == QEvent::FocusOut ){
        for(int i = 0; i < 3; i++){
            QString s = ed[i]->text();
            int pos = 0;

            if( ed[i]->validator()->validate( s, pos ) !=
                QValidator::Acceptable )
            {
                ed[i]->setText( lastValid[i] );
            } else {
                lastValid[i] = ed[i]->text();
            }
        }
    }
    return QFrame::eventFilter( o, e );;
}

/*!

  Layout the arrow buttons.
 */
void QDateTimeEditBase::layoutArrows()
{
    QSize bs;
    if ( style() == WindowsStyle )
        bs.setHeight( height()/2 - frameWidth() );
    else
        bs.setHeight( height()/2 );
    if ( bs.height() < 8 )
        bs.setHeight( 8 );
    bs.setWidth( bs.height() * 8 / 5 ); // 1.6 - approximate golden mean

    int y = style() == WindowsStyle ? frameWidth() : 0;
    int x, lx;
    if ( QApplication::reverseLayout() ) {
        x = y;
        lx = x + bs.width() + frameWidth();
    } else {
        x = width() - y - bs.width();
        lx = frameWidth();
    }

    if ( style() == WindowsStyle )
        setFrameRect( QRect( 0, 0, 0, 0 ) );
    else
        setFrameRect( QRect( lx - frameWidth(), 0, width() - bs.width(),
                             height() ) );

    if ( up->size() != bs || down->size() != bs ) {
        up->resize( bs );
        down->resize( bs );
        updateArrows();
    }

    up->move( x, y );
    down->move( x, height() - y - up->height() );
}



/*!
  \class QDateEdit qdatetimeedit.h
  \brief The QDateEdit class provides a combined line edit box and spin box to edit dates

    The QDateEdit class provides a combined line edit box and spin box to edit dates

    QDateEdit allows the user to edit the date by using the keyboard or
    the arrow keys to increase/decrease date values.  The Tab key can be
    used to move from field to field within the QDateEdit box.

    If illegal values are entered, they will be reverted to the last
    known legal value. For example if the user enters 5000 for the day
    value, and it was 12 before they stated editing, the value will be
    reverted to 12.

 */


/*!

  Constructs an empty QDateEdit widget.
 */
QDateEdit::QDateEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
}

/*!

  Constructs a QDateEdit widget and initializes it with the QDate \a d.
 */
QDateEdit::QDateEdit( const QDate & d, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setDate( d );
}

/*!

  \internal Initialization.
 */
void QDateEdit::init()
{
    setDateSeparator( "-" );
    setOrder( "YMD" ); // ## ISO default?
    connect( this, SIGNAL( valueChanged() ), this, SLOT( someValueChanged() ) );
}

void QDateEdit::someValueChanged()
{
    emit valueChanged( date() );
}


/*!

  Set the date in this QDateEdit.
 */
void QDateEdit::setDate( const QDate & d )
{
    QDate oldDate = date();
    QIntValidator * v[3];
    int yy = d.year();
    int mm = d.month();
    int dd = d.day();

    v[0] = (QIntValidator *) ed[0]->validator();
    v[1] = (QIntValidator *) ed[1]->validator();
    v[2] = (QIntValidator *) ed[2]->validator();

    if( (yy > v[yearPos]->top()) || (yy < v[yearPos]->bottom()) ||
        (mm > v[monthPos]->top()) || (mm < v[monthPos]->bottom()) ||
        (dd > v[dayPos]->top()) || (dd < v[dayPos]->bottom()) )
    {
        // Date out of range - leave it blank
        ed[0]->setText( "" );
        ed[1]->setText( "" );
        ed[2]->setText( "" );
    } else {
        ed[yearPos]->setText( QString::number( yy ) );
        ed[monthPos]->setText( QString::number( mm ) );
        ed[dayPos]->setText( QString::number( dd ) );
    }
    if ( oldDate != date() )
        emit valueChanged( d );
//    ed[0]->setFocus();
//    ed[0]->selectAll();
}

/*!

  Returns the date in this QDateEdit.
 */
QDate QDateEdit::date() const
{
    ((QDateEdit *) this)->fixup(); // Fix invalid dates

    return QDate( ed[yearPos]->text().toInt(), ed[monthPos]->text().toInt(),
                  ed[dayPos]->text().toInt() );
}

/*! \fn void valueChanged( const QDate& )

  This signal is emitted every time the date changes.  The argument is
  the new date.
*/

/*!

  Set the order in which each part of the date should appear in the edit
  box.

  The year is signified with 'Y', the month with 'M' and the day with
  'D'. For the US the format would probably be 'MDY', for Japan, 'YMD',
  for Europe, 'DMY'.

  \sa order
*/
void QDateEdit::setOrder( const QString & fmt )
{
    QString tmp;

    if( fmt.length() > 3 ) return;
    tmp = fmt.upper();

    if( !tmp.contains( 'Y' ) || !tmp.contains( 'M' ) || !tmp.contains( 'D' ) )
        return;

    yearPos  = tmp.find( 'Y' );
    monthPos = tmp.find( 'M' );
    dayPos   = tmp.find( 'D' );

    ed[yearPos]->setRange( 1753, 3000 );
    ed[monthPos]->setRange( 1, 12 );
    ed[dayPos]->setRange( 1, 31 );

    ed[yearPos]->setMaxLength( 4 );
    ed[monthPos]->setMaxLength( 2 );
    ed[dayPos]->setMaxLength( 2 );

    format[yearPos]  = 'Y';
    format[monthPos] = 'M';
    format[dayPos]   = 'D';
    format[3] = 0;
}

/*!

  Returns a string that indicates the order in which each part of the
  date appears in the editor.

  The year is signified with 'Y', the month with 'M' and the day with
  'D'. If the string 'MDY' was returned this would signify a month, day,
  year ordering.

  \sa setOrder
*/
QString QDateEdit::order() const
{
    return format;
}

/*!
  Set the separator string for this date editor.
 */
void QDateEdit::setDateSeparator( const QString & s )
{
    separator = s;
    sep[0]->setText( separator );
    sep[1]->setText( separator );
}

/*!
  Returns the separator string for this date editor.
 */
QString QDateEdit::dateSeparator() const
{
    return separator;
}

/*!
  \internal
  Post-process the edited date. This will guarantee that a date is
  valid.
*/
void QDateEdit::fixup()
{
    int yy, mm, dd;
    QIntValidator * v[3];
    v[0] = (QIntValidator *) ed[0]->validator();
    v[1] = (QIntValidator *) ed[1]->validator();
    v[2] = (QIntValidator *) ed[2]->validator();

    yy = ed[yearPos]->text().toInt();
    mm = ed[monthPos]->text().toInt();
    dd = ed[dayPos]->text().toInt();

    if( !QDate::isValid( yy, mm, dd) ){
        if( !QDate::isValid( yy, 1, 1 ) )
            if( yy > v[yearPos]->top() ) yy = v[yearPos]->top();
            else if( yy < v[yearPos]->bottom() ) yy = v[yearPos]->bottom();
        if( !QDate::isValid( yy, mm, 1 ) )
            if( mm > v[monthPos]->top() ) mm = v[monthPos]->top();
            else if( mm < v[monthPos]->bottom() ) mm = v[monthPos]->bottom();
        if( dd > v[dayPos]->top() ) dd = v[dayPos]->top();
        else if( dd < v[dayPos]->bottom() ) dd = v[dayPos]->bottom();

        while( !QDate::isValid( yy, mm, dd ) ){
            dd--;
        }
        ed[yearPos]->setText( QString::number( yy ) );
        ed[monthPos]->setText( QString::number( mm ) );
        ed[dayPos]->setText( QString::number( dd ) );
    }
}

/*! \reimp
*/
bool QDateEdit::event( QEvent* e )
{
    switch ( e->type() ) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = (QKeyEvent*)e;
        QDate newDate = date();
        if ( ke->key() == Key_Tab || ke->key() == Key_BackTab ) {
            if ( newDate != oldDate ) {
                emit valueChanged( newDate );
            }
        } else if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
            if ( newDate != oldDate )
                emit valueChanged( newDate );
        }
    }
    break;
    default:
        break;
    }
    return QDateTimeEditBase::event( e );
}


/*! \reimp
*/
void QDateEdit::resizeEvent( QResizeEvent * )
{
    layoutArrows();

    int fw       = frameWidth();
    int h        = height() - fw*2;
    int numWidth = QFontMetrics(font()).width("X");
    int offset   = width() - up->width() - fw;
    int sepWidth = QFontMetrics(font()).width( separator );

    ed[yearPos]->resize( numWidth*4, h );
    ed[monthPos]->resize( numWidth*2, h );
    ed[dayPos]->resize( numWidth*2, h );

    // ### alignment?
    ed[2]->move( offset - ed[2]->width(), fw );
    ed[1]->move( ed[2]->x() - ed[1]->width() - sepWidth, fw );
    ed[0]->move( ed[1]->x() - ed[0]->width() - sepWidth, fw );

    sep[0]->resize( sepWidth, h - 2);
    sep[1]->resize( sepWidth, h - 2);
    sep[0]->move( ed[0]->x() + ed[0]->width() + 1, fw );
    sep[1]->move( ed[1]->x() + ed[1]->width() + 1, fw );
}

/*!

  \class QTimeEdit qdatetimeedit.h
  \brief The QTimeEdit class provides a combined line edit box and spin box to edit times

  The QTimeEdit class provides a combined line edit box and spin box to edit times

  QTimeEdit allows the user to edit the time by using the
  keyboard or the arrow buttons to increase/decrease time
  values.  The Tab key can be used to move from field to field within
  the QTimeEdit box.

  If illegal values are entered, these will be reverted to the last
  known legal value. For example if the user entered 5000 for the hour
  value, and it was 12 before they stated editing, the value will be
  reverted to 12.
  */

/*!

  Constructs an empty QTimeEdit widget.
*/
QTimeEdit::QTimeEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
}

/*!

  Constructs a QTimeEdit widget, and initializes it with the QTime \a t.
*/
QTimeEdit::QTimeEdit( const QTime & t, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setTime( t );
}

void QTimeEdit::someValueChanged()
{
    emit valueChanged( time() );
}

/*!

  \internal Initialization.
 */
void QTimeEdit::init()
{
    ed[0]->setRange( 0, 23 );
    ed[1]->setRange( 0, 59 );
    ed[2]->setRange( 0, 59 );
    setTimeSeparator( ":" );
    connect( this, SIGNAL( valueChanged() ), this, SLOT( someValueChanged() ) );
}

/*!

  Set the time in this QTimeEdit.
 */
void QTimeEdit::setTime( const QTime & t )
{
    QTime oldTime = time();
    ed[0]->setText( QString::number( t.hour() ) );
    ed[1]->setText( QString::number( t.minute() ) );
    ed[2]->setText( QString::number( t.second() ) );
    if ( oldTime != time() )
        emit valueChanged( t );

//    ed[0]->setFocus();
//    ed[0]->selectAll();
}

/*!

  Returns the time in this QTimeEdit.
 */
QTime QTimeEdit::time() const
{
    return QTime( ed[0]->text().toInt(), ed[1]->text().toInt(),
                  ed[2]->text().toInt() );
}

/*! \fn void valueChanged( const QTime& )

    This signal is emitted every time the time changes.  The argument is
    the new time.
*/


/*!
  Set the separator string for this time editor.
 */
void QTimeEdit::setTimeSeparator( const QString & s )
{
    separator = s;
    sep[0]->setText( separator );
    sep[1]->setText( separator );
}

/*!
  Returns the separator string for this time editor.
 */
QString QTimeEdit::timeSeparator() const
{
    return separator;
}

/*! \reimp
*/
bool QTimeEdit::event( QEvent* e )
{
    switch ( e->type() ) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = (QKeyEvent*)e;
        QTime newTime = time();
        if ( ke->key() == Key_Tab || ke->key() == Key_BackTab ) {
            if ( newTime != oldTime )
                emit valueChanged( newTime );
        } else if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
            if ( newTime != oldTime )
                emit valueChanged( newTime );
        }
    }
    break;
    default:
        break;
    }
    return QDateTimeEditBase::event( e );
}


/*! \reimp
 */
void QTimeEdit::resizeEvent( QResizeEvent * )
{
    layoutArrows();

    int fw       = frameWidth();
    int h        = height() - fw*2;
    int numWidth = QFontMetrics(font()).width("X");
    int offset   = width() - up->width() - fw;
    int sepWidth = QFontMetrics(font()).width( separator );

    ed[0]->resize( numWidth*2, h );
    ed[1]->resize( numWidth*2, h );
    ed[2]->resize( numWidth*2, h );

    // ### alignment?
    ed[2]->move( offset - ed[2]->width(), fw );
    ed[1]->move( ed[2]->x() - ed[1]->width() - sepWidth, fw );
    ed[0]->move( ed[1]->x() - ed[0]->width() - sepWidth, fw );

    sep[0]->resize( sepWidth, h - 2);
    sep[1]->resize( sepWidth, h - 2);
    sep[0]->move( ed[0]->x() + ed[0]->width() + 1, fw );
    sep[1]->move( ed[1]->x() + ed[1]->width() + 1, fw );
}


/*!

  \class QDateTimeEdit qdatetimeedit.h
  \brief The QDateTimeEdit class provides a combined line edit box and spin box to edit datetimes

  The QDateTimeEdit class provides a combined line edit box and spin box to edit datetimes

  QDateTimeEdit consists of a QDateEdit and QTimeEdit widget placed side by
  side and offers the functionality of both.

  \sa QDateEdit QTimeEdit
  */

/*!

  Constructs an empty QDateTimeEdit widget.
*/
QDateTimeEdit::QDateTimeEdit( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
}

/*!

  Constructs a QDateTimeEdit widget, and initializes it with the QDateTime \a dt.
*/
QDateTimeEdit::QDateTimeEdit( const QDateTime & dt, QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
    setDateTime( dt );
}


void QDateTimeEdit::resizeEvent( QResizeEvent * )
{
    layoutEditors();
}

QSize QDateTimeEdit::minimumSizeHint() const
{
    return de->minimumSizeHint() + te->minimumSizeHint();
}

void QDateTimeEdit::layoutEditors()
{
    int h        = height() - frameWidth()*2;
    int numWidth = (width() - frameWidth()*2) / 2;
    int fw       = frameWidth();

    de->resize( numWidth, h );
    te->resize( numWidth, h );

    de->move( fw, fw );
    te->move( de->x() + de->width() + fw, fw );
}

/*!  \internal
 */

void QDateTimeEdit::init()
{
    de = new QDateEdit( this );
    te = new QTimeEdit( this );
    connect( de, SIGNAL( valueChanged( const QDate& ) ),
             this, SLOT( newValue( const QDate& ) ) );
    connect( te, SIGNAL( valueChanged( const QTime& ) ),
             this, SLOT( newValue( const QTime& ) ) );
    setFocusProxy( de );
    layoutEditors();
}

QSize QDateTimeEdit::sizeHint() const
{
    return de->sizeHint() + te->sizeHint();
}

/*!  Set the datetime in this QDateTimeEdit.
 */

void QDateTimeEdit::setDateTime( const QDateTime & dt )
{
    de->setDate( dt.date() );
    te->setTime( dt.time() );
}

/*!  Returns the datetime in this QDateTimeEdit.
 */

QDateTime QDateTimeEdit::dateTime() const
{
    return QDateTime( de->date(), te->time() );
}

/*!  Set the separator for the date in this QDateTimeEdit.
 */
void QDateTimeEdit::setDateSeparator( const QString & s )
{
    de->setDateSeparator( s );
}

/*!  Returns the separator for the date in this QDateTimeEdit.
 */
QString QDateTimeEdit::dateSeparator() const
{
    return de->dateSeparator();
}

/*!  Set the separator for the time in this QDateTimeEdit.
 */
void QDateTimeEdit::setTimeSeparator( const QString & s )
{
    te->setTimeSeparator( s );
}

/*!  Returns the separator for the time in this QDateTimeEdit.
 */
QString QDateTimeEdit::timeSeparator() const
{
    return te->timeSeparator();
}

/*! \intern
 */

void QDateTimeEdit::newValue( const QDate& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}

/*! \intern
 */

void QDateTimeEdit::newValue( const QTime& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}



#endif
