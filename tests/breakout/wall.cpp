#include "wall.h"
#include <stdlib.h>
#include <math.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfile.h>

// game items

Ball::Ball( GameMain *g ) :
    QCanvasEllipse( g->getBrickHeight()/2, g->getBrickHeight()/2, g->getCanvas() ), game( g )
{
    setBrush( game->getBallColor() );
    setAnimated( FALSE );
}


void Ball::advance( int phase )
{
    switch ( phase ) {
        case 0: {
	    double vx =	xVelocity();
	    double vy =	yVelocity();
	    if ( vy == 0 ) { // must not be 0
		vy = speed;
		vx = 0;
		angle = Pi / 2;
	    }
	    double newVx = vx;
	    double newVy = vy;
	    
	    // collision detection
	    QRect rect( boundingRect() );
	    QCanvasItemList cil = canvas()->collisions( rect );
	    for ( QCanvasItemList::Iterator it = cil.begin(); it != cil.end(); ++it) {
		QCanvasItem *item = *it;
		QRect itemRect( item->boundingRect() );
		switch ( item->rtti() ) {
		    case Pad::RTTI: {
			newVy = - fabs( vy );
			angle = atan2( - newVy, newVx );
			if ( angle < 0 )
			    angle += 2 * Pi;
			if ( rect.center().x() < itemRect.left() + itemRect.width() / 3 ) {
			    angle += Pi / 8;
			} else if ( rect.center().x() > itemRect.left() + itemRect.width() * 2 / 3 ) {
			    angle -= Pi / 8;
			}
			if ( angle > Pi * 15 / 16 )
			    angle = Pi * 15 / 16;
			if ( angle < Pi / 16 )
			    angle = Pi / 16;
			newVx = speed * cos( angle );
			newVy = - speed * sin( angle );
			break;
		    }
		    case Brick::RTTI: {
			if ( vx == 0 ) {
			    newVy = - vy;
			} else {
			    double ly = ( vy > 0 ) ?
				( rect.bottom() - itemRect.top() ) :
				( rect.top() - itemRect.bottom() );
			    double lx = ( ly / vy ) * vx;
			    // translate ball backwards until y coordinate
			    // penetrates the object ( -lx, -ly )
			    if ( ( rect.right() - lx <= itemRect.left() ) ||
				 ( rect.left() - lx >= itemRect.right() )    )
				newVx = - vx;
			    if ( ( rect.right() - lx >= itemRect.left() ) &&
				 ( rect.left() - lx <= itemRect.right() )    )
				newVy = - vy;
			}
			( (Brick*)item )->hit();
			break;
		    }
		    case DeathLine::RTTI: {
			( (DeathLine*)item )->hit( this );
			return;
		    }			     
		} 
	    }

	    if ( ( rect.left() < 0 && vx < 0 ) ||
		 ( rect.right() >= canvas()->width() && vx > 0 ) )
		newVx = - vx;
	    if ( ( rect.top() < 0 && vy < 0 ) ||
		 ( rect.bottom() >= canvas()->height() && vy > 0 ) )
		newVy = - vy;
	    angle = atan2( - newVy, newVx );
	    if ( angle < 0 )
		angle += 2 * Pi;
	    setSpeed( angle, speed );
	    break;
	}
        case 1:
	    QCanvasItem::advance( phase );
	break;
    }
}

void Ball::setSpeed( double ang, double sp ) 
{
    angle = ang;
    speed = sp;
    setVelocity( speed * cos( angle ), - speed * sin( angle ) );
}

Pad::Pad( GameMain *g ) :
    QCanvasRectangle( 0, 0, 2*g->getBrickWidth(), g->getBrickHeight()/2 , g->getCanvas() ), game( g )
{
    setBrush( game->getPadColor() );
    move( ( g->getBrickCols()/2 - 1 ) * g->getBrickWidth(), ( BricksVertical - 1.5 ) * g->getBrickHeight());
}


Brick::Brick( GameMain *g, int x, int y, const QColor& col, int score ) :
    QCanvasRectangle( x * g->getBrickWidth(), y * g->getBrickHeight(), g->getBrickWidth(), g->getBrickHeight(), g->getCanvas() ), game( g ),
    color( col), points( score )
{
    setBrush( QBrush( color, QCanvasRectangle::SolidPattern ) );
    int w = width();
    int dw = w/10 + 1;
    int h = height();
    int dh = h/10 + 1;
    setSize( w - dw, h - dh);
    moveBy( dw/2, dh/2 );
}

int Brick::bricksToDestroy = 0;

Brick::~Brick()
{
    if ( !game->getRunning() )
	return;
    QRect rect = boundingRect();
    int x = rect.center().x();
    int y = rect.center().y();
    int w = rect.width();
    int h = rect.height();
    TempItems *timer = new TempItems( 500, canvas() );
    for ( int i = 2 + rand() % 3; i > 0; --i ) {
	QCanvasRectangle *particle = new QCanvasRectangle( x, y, w/2, h/2, canvas() );
	void *p = particle;
	particle->setAnimated( TRUE );
	int v = qRound( game->getMaxBallSpeed() );
	particle->setVelocity( rand() % (2*v) - v, rand() % (2*v) - v);
        particle->setBrush( QBrush( color ) );
	particle->show();
	timer->addItem( particle );
    }
    timer->start();
    if ( points ) {
	QFont font;
	font.setBold( TRUE );
	font.setPixelSize( h );
	QCanvasText *text = new QCanvasText( QString( "%1" ).arg( points ), font, canvas() );
	text->setColor( color );
        QRect rectText = text->boundingRect();
	text->move( x - rectText.width() / 2, y - rectText.height() / 2 );
	text->show();
	new TempItems( text, 1000 );
    }
    if ( bricksToDestroy <= 0 ) {
	emit allDestroyed();
    }
}

void Brick::hit()
{
    emit score( points );
    delete this;
}

StandardBrick::StandardBrick( GameMain *game, int x, int y, const QColor& col, int score ) :
    Brick( game, x, y, col, score )
{
    ++bricksToDestroy;
}

StoneBrick::StoneBrick( GameMain *game, int x, int y, const QColor& col, int score ) :
    Brick( game, x, y, col, score )
{
    setBrush( QBrush( color, QCanvasRectangle::Dense5Pattern ) );
}

void StoneBrick::hit()
{
}

HardBrick::HardBrick( GameMain *game, int x, int y, const QColor& col, int hits, int score ) :
    Brick( game, x, y, score ), hitsLeft( hits )
{
    color = col;
    paintIt();
}

void HardBrick::hit()
{
    if ( --hitsLeft <= 0 )
	Brick::hit();
    else
	paintIt();
}

void HardBrick::paintIt()
{
    switch ( hitsLeft ) {
	case 1: {
	    setBrush( QBrush( color, QCanvasRectangle::SolidPattern ) );
	    break;
	}
	case 2:  {
	    setBrush( QBrush( color, QCanvasRectangle::Dense1Pattern ) );
	    break;
	}
	case 3: {
	    setBrush( QBrush( color, QCanvasRectangle::Dense2Pattern ) );
	    break;
	}
	case 4: {
	    setBrush( QBrush( color, QCanvasRectangle::Dense3Pattern ) );
	    break;
	}
	default: setBrush( QBrush( color, QCanvasRectangle::Dense4Pattern ) );
    }
    update();
}


DeathLine::DeathLine( GameMain *g ) :
    QCanvasLine( g->getCanvas() ), game( g )
{
    setPen( QPen( QColor(), g->getBrickHeight() / 2, QCanvasLine::NoPen) );
    setPoints( 0, ( BricksVertical ) * g->getBrickHeight(), g->getBrickCols() * g->getBrickWidth() - 1, ( BricksVertical ) * g->getBrickHeight() );
}

void DeathLine::hit( Ball *ball)
{
    delete ball;
    emit killedBall();
}

Lives::Lives( GameMain *g, int lives ) :
    QCanvasText( g->getCanvas() ), game( g ), number( lives )
{
    QFont font;
    font.setBold( TRUE );
    font.setPixelSize( g->getBrickHeight() );
    setFont( font );
    setColor( game->getTextColor() );
    move( g->getBrickWidth(), ( BricksVertical - 1 ) * g->getBrickHeight() );
    print();
}

void Lives::looseLife()
{
    if ( number > 0 )
	--number;
    print();
    if ( number <= 0) 
	emit noMoreLives();
    else 
	emit newBall();
}

void Lives::print()
{
    setText( QString( "Lives: %1" ).arg( number ) );
    update();
}

Score::Score( GameMain *g ) :
    QCanvasText( g->getCanvas() ), game( g ), number( 0 )
{
    QFont font;
    font.setBold( TRUE );
    font.setPixelSize( g->getBrickHeight() );
    setFont( font );
    setColor( game->getTextColor() );
    move( ( g->getBrickCols() - 5 ) * g->getBrickWidth(), ( BricksVertical - 1 ) * g->getBrickHeight() );
    print();
}
    
void Score::addScore( int value )
{
    number += value;
    if ( number < 0 )
	number = 0;
    print();
}

void Score::print()
{
    setText( QString( "Score: %1" ).arg( number, 5 ) );
    update();
}


TempItems::TempItems( int ms, QCanvas *c ) :
    time( ms ), canvas( c )
{
}

TempItems::TempItems( QCanvasItem *item, int ms ) :
    time( ms ), canvas( item->canvas() )
{
    addItem( item );
    start();
}

void TempItems::addItem( QCanvasItem *item )
{
    list.append( item );
}

void TempItems::start()
{
    QTimer::singleShot( time, this, SLOT(stop()) );
}

void TempItems::stop() {
    QCanvasItemList::iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	void *p = *it;
	delete *it;
    }
    delete this;
}

//---------------------------------------------------------------------

Level::Level( GameMain *g, const QString &filename, const QString &startTag ) :
    game( g ), levelFile( filename ), levelStartTag( startTag )
{
    levels.setAutoDelete( TRUE );
    load();
}

void Level::load( const QString &filename ) 
{
    if ( !filename.isNull() )
	levelFile = filename;
    levels.clear();
    QFile f( levelFile );
    if ( !f.open( IO_ReadOnly ) ) {
	// no level file
	return;
    }
    QTextStream t( &f );
    QStringList *lines;
    QString line;
    while ( !( line = t.readLine() ).isNull() ) {
	if ( line.contains( levelStartTag, FALSE ) ) {
	    lines = new QStringList;
	    while ( !( line = t.readLine() ).isNull() && !line.contains( LevelEndTag, FALSE ) ) 
		lines->append( line );
	    if ( !line.isNull() )  //if ( line.contains(..) )
		levels.append( lines );
	    else
		delete lines;
	}
    }
    levels.first();
}

void Level::start()
{
    if ( levels.first() )
	createLevel();
}

void Level::next()
{
    if ( levels.next() )
	createLevel();
    else
	emit noMoreLevels();
}

void Level::createLevel()
{
    QStringList *level = levels.current();
    if ( !level )
	return;
    Brick *brick;
    char brickType;
    char brickColor;
    int row = 0;
    for ( QStringList::Iterator it = level->begin(); 
	    it != level->end() && row < game->getBrickRows() ; ++it, ++row ) {
	QString &line = (*it);
	for ( int col = 0; 
		col < QMIN( game->getBrickCols(), (int)line.length() / 2 ); ++col ) {
	    brickType = line[2 * col].upper();
	    brickColor = line[2 * col + 1].upper();
	    QColor color;
	    switch( brickColor ) {
		case 'R': {
		    color = QColor( "red" );
		    break;
		}
		case 'G': {
		    color = QColor( "green" );
		    break;
		}
		case 'B': {
		    color = QColor( "blue" );
		    break;
		}
		case 'C': {
		    color = QColor( "cyan" );
		    break;
		}
		case 'M': {
		    color = QColor( "magenta" );
		    break;
		}
		case 'Y': {
		    color = QColor( "yellow" );
		    break;
		}
		case 'S': {
		    color = QColor( "gray" );
		    break;
		}
	    }
	    if ( color.isValid() ) {
		brick = 0;
		switch( brickType ) {
		    case 'B': {
			brick = new StandardBrick( game, col, row, color, BrickPoints );
			break;
		    }
		    case '1': case '2': case '3': case '4': case '5': 
		    case '6': case '7': case '8': case '9':
		    {
			int hits = brickType - '0';
			brick = new HardBrick( game, col, row, color, hits, hits * BrickPoints );
			break;
		    }
		    case 'S': case '0': {
			brick = new StoneBrick( game, col, row, color, 10 * BrickPoints );
			break;
		    }

		}
		if ( brick ) {
		    connect( brick, SIGNAL(score(int)), game->getScore(), SLOT(addScore(int)) );
		    connect( brick, SIGNAL(allDestroyed()), game, SLOT(endLevel()) );
		    brick->show();
		}
	    
	    } //if ( color.isValid() )
	} //for int col
    } //for QStringList::Iterator it
}


TableView::TableView( GameMain *parent, const char *name, WFlags f ) :
    QCanvasView( parent->getCanvas(), parent, name, f ), game( parent )
{
    setFocusPolicy(QWidget::StrongFocus);
    takeMouse( TRUE );
    setFocus();
}

void TableView::contentsMouseMoveEvent( QMouseEvent* me)
{
    int x = me->pos().x();
    Pad *pad = game->getPad();
    if ( x < 0 )
	x = 0;
    if ( x + pad->boundingRect().width() > game->getCanvas()->width() ) 
	x = game->getCanvas()->width() - pad->boundingRect().width();
    pad->setX( x );
    if ( game->getGluedBall() ) {
	game->getBall()->setX( x );
    }
}

void TableView::contentsMousePressEvent( QMouseEvent *me)
{
    if ( !mouseTaken ) {
	takeMouse( TRUE );
    } else if ( game->getGluedBall() ) {
	game->setGluedBall( FALSE );
    }
}

void TableView::takeMouse( bool take )
{
    if ( take ) {
	viewport()->setMouseTracking( TRUE );
	setCursor( Qt::BlankCursor );
	viewport()->grabMouse();
	mouseTaken = TRUE;
        //setFocus();
    } else {
	viewport()->releaseMouse();
	viewport()->setMouseTracking( FALSE );
	setCursor( Qt::ArrowCursor );
	mouseTaken = FALSE;
    }
}

void TableView::keyPressEvent ( QKeyEvent *e )
{
    takeMouse( FALSE );
}

void TableView::focusOutEvent( QFocusEvent * e )
{
    if ( mouseTaken )
	takeMouse( FALSE );
}



GameMain::GameMain( int cols, int rows, int width, int height, QWidget *parent, const char *name, WFlags f ) :
    QMainWindow( parent, name, f )
{
    brickCols = cols;
    brickRows = rows;
    brickWidth = width;
    brickHeight = height;

    canvas = new QCanvas( brickCols * brickWidth, brickRows * brickHeight );
    advancePeriod = 20;
    #ifdef Q_OS_WIN32
    if ( QApplication::winVersion() != Qt::WV_NT_based )
	advancePeriod = 55;
    #endif
    canvas->setAdvancePeriod( advancePeriod );
    maxBallSpeed = 0.35 * brickHeight * advancePeriod / 20.0; //because of Win98 
    
    tableView = new TableView( this );
    setCentralWidget( tableView );

    inverseColors = FALSE;
    setInverseColors( TRUE );
    running = FALSE;
    gluedBall = TRUE;

    QMenuBar *menu = menuBar();

    QPopupMenu *gameMenu = new QPopupMenu( menu );
    gameMenu->insertItem( "&New Game", this, SLOT(newGame()), CTRL+Key_N );
    gameMenu->insertSeparator();
    gameMenu->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
    menu->insertItem( "&Game", gameMenu );

    optionsMenu = new QPopupMenu( menu );
    colors_id = optionsMenu->insertItem( "Inversed &Colors", this, SLOT(toggleColors()), CTRL+Key_C );
    optionsMenu->setItemChecked(colors_id, TRUE);
    //optionsMenu->insertSeparator();
    menu->insertItem( "&Options", optionsMenu );
    
    QPopupMenu *helpMenu = new QPopupMenu( menu );
    helpMenu->insertItem( "&Help", this, SLOT(help()), Key_F1 );
    helpMenu->insertSeparator();
    helpMenu->insertItem( "&About Qt", this, SLOT(about()), CTRL+Key_A );
    menu->insertItem( "&Help", helpMenu );

    setFocusPolicy(QWidget::NoFocus);

    level = new Level( this, LevelFile, LevelBeginTag );
    connect( level, SIGNAL(noMoreLevels()), this, SLOT(endGame()) );

    newGame();
}

GameMain::~GameMain()
{
    delete tableView;
    delete canvas; // deletes all QCanvasItems as well
}

void GameMain::newGame()
{
    running = FALSE;
    QCanvasItemList cil = canvas->allItems();
    for (QCanvasItemList::Iterator it = cil.begin(); it != cil.end(); ++it) {
	if ( *it )
	    delete *it;
    }

    lives = new Lives( this, 3 );
    lives->show();

    score = new Score( this );
    score->show();

    pad = new Pad( this );
    pad->show();

    level->start();

    deathLine = new DeathLine( this );
    deathLine->show();

    ball = 0;
    connect( deathLine, SIGNAL(killedBall()), lives, SLOT(looseLife()) );
    connect( lives, SIGNAL(newBall()), this, SLOT(startBall()) );
    connect( lives, SIGNAL(noMoreLives()), this, SLOT(endGame()) );
    
    tableView->show();

    startBall();
    running = TRUE;
}

void GameMain::endGame()
{

}

void GameMain::endLevel()
{
    level->next();    
}

void GameMain::startBall()
{
    ball = new Ball( this );
    setGluedBall( TRUE );
    int x = pad->boundingRect().center().x();
    int y = pad->boundingRect().top() - ball->height() / 2;
    ball->move( x, y );
    //ball->move( 100, ( BricksVertical - 3 ) * brickHeight );
    ball->setSpeed( 0.25 * Pi, maxBallSpeed / 2 );
    ball->show();
}

void GameMain::help()
{
    static QMessageBox* about = new QMessageBox( "QtWall Help",
        "<h2>QtWall</h2>"
        "<h3>Objectives:</h3>"
        "move the pad, bounce the ball, destroy all bricks!"
	    , QMessageBox::Information, 1, 0, 0, this, 0, FALSE );
    about->setButtonText( 1, "OK" );
    about->show();
}

void GameMain::about()
{
    QMessageBox::aboutQt( this, "About Qt" );
}

void GameMain::setInverseColors( bool inver )
{
    if ( inver == inverseColors )
	return;
    inverseColors = inver;
    if ( inverseColors ) {
        canvas->setBackgroundColor( QColor( "black" ) );
	padColor = QColor( "blue" );
	ballColor = QColor( "white" );
	textColor = QColor( "white" );
    } else {
        canvas->setBackgroundColor( QColor( "white" ) );
	padColor = QColor( "blue" );
	ballColor = QColor( "black" );
	textColor = QColor( "black" );
    }
    QCanvasItemList cil = canvas->allItems();
    for ( QCanvasItemList::Iterator it = cil.begin(); it != cil.end(); ++it) {
	QCanvasItem *item = *it;
	switch ( item->rtti() ) {
	    case Ball::RTTI: {
		( (Ball*)item )->setBrush( ballColor );
		break;
	    }
	    case Pad::RTTI: {
		( (Pad*)item )->setBrush( padColor );
		break;
	    }
	    case Lives::RTTI: {
		( (Lives*)item )->setColor( textColor );
		break;
	    }
	    case Score::RTTI: {
		( (Score*)item )->setColor( textColor );
		break;
	    }
	}
    }
    canvas->update();
}

void GameMain::toggleColors()
{
    bool inv = !optionsMenu->isItemChecked( colors_id );
    optionsMenu->setItemChecked( colors_id, inv );
    setInverseColors( inv );
}

void GameMain::setGluedBall( bool glued )
{
    gluedBall = glued;
    if ( ball )
	ball->setAnimated( !glued );
}


void GameMain::keyPressEvent ( QKeyEvent *e )
{
    tableView->setFocus();
}

