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
#include <qsocket.h>
#include <qserversocket.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>


// game items

Ball::Ball( GameMain *g, Player *pl ) :
    QCanvasEllipse( g->getBrickHeight()/2, g->getBrickHeight()/2, g->getCanvas() ), 
    game( g ), owner( pl )
{
    setBrush( game->getBallColor() );
    stop();
    emit ballOwnedBy( owner );
}

void Ball::start()
{
    setAnimated( TRUE );
    if ( game->isMyBall() )
	emit ballPosition( (double)x() / game->getBrickWidth(),
			   (double)y() / game->getBrickHeight(),
		           angle, speed );
}

void Ball::stop()
{
    setAnimated( FALSE );
}

void Ball::advance( int phase )
{
    if ( game->isGluedBall() ) {
	stop();
	putOnPad();
	return;
    }
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
			Pad *pad = (Pad*)item;
			PlayerPosition position = pad->getPlayer()->getPosition();
			if ( position == PlayerDown ) {
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
			} else { //up
			    newVy = fabs( vy );
			    angle = atan2( - newVy, newVx );
			    if ( angle < 0 )
				angle += 2 * Pi;
			    if ( rect.center().x() < itemRect.left() + itemRect.width() / 3 ) {
				angle -= Pi / 8;
			    } else if ( rect.center().x() > itemRect.left() + itemRect.width() * 2 / 3 ) {
				angle += Pi / 8;
			    }
			    if ( angle > Pi * 31 / 16 )
				angle = Pi * 31 / 16;
			    if ( angle < Pi * 17 / 16 )
				angle = Pi *17 / 16;
			}
			if ( pad->getPlayer() != owner ) {
			    owner = pad->getPlayer();
			    emit ballOwnedBy( owner );
			}
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
			Brick *brick = (Brick*)item;
			if ( game->isMyBall() ) {
         		    connect( brick, SIGNAL(score(int)), game->getMyScore(), SLOT(addScore(int)) );
    			    brick->hit();
			} else 
			    disconnect( brick, SIGNAL(score(int)) );
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
	    if ( ( newVx != vx ) || ( newVy != vy ) ) {
		angle = atan2( - newVy, newVx );
		if ( angle < 0 )
		    angle += 2 * Pi;
		setSpeed( angle, speed );
		emit ballPosition( ( (double)x() + newVx ) / game->getBrickWidth(),
		                   ( (double)y() + newVy ) / game->getBrickHeight(),
				   angle, speed );
	    }  
	    break;
	}
        case 1:
	    QCanvasItem::advance( phase );
	    break;
    }
}

void Ball::setBall( double x, double y, double ang, double sp )
{
/*    if ( xpos + boundingRect().width() > game->getCanvas()->width() ) 
	xpos = game->getCanvas()->width() - boundingRect().width();*/
    move( x * game->getBrickWidth(), y * game->getBrickHeight() );
    setSpeed( ang, sp );
    game->setGluedBall( FALSE );
}

void Ball::setSpeed( double ang, double sp ) 
{
    angle = ang;
    speed = sp;
    setVelocity( speed * cos( angle ), - speed * sin( angle ) );
}

void Ball::setOwner( Player *player )
{
    owner = player;
}

void Ball::putOnPad()
{
    int x;
    int y;
    if ( owner->getPosition() == PlayerDown ) {
	x = game->getPad( owner )->boundingRect().center().x();
	y = game->getPad( owner )->boundingRect().top() - height() / 2;
    } else { //up
	x = game->getPad( owner )->boundingRect().center().x();
	y = game->getPad( owner )->boundingRect().bottom() + height() / 2;
    }
    move( x, y );
}


Pad::Pad( GameMain *g, Player *pl ) :
    QCanvasRectangle( 0, 0, 2*g->getBrickWidth(), g->getBrickHeight()/2 , g->getCanvas() ), 
    game( g ), player ( pl )
{
    oldPosition = 0;
    setBrush( game->getPadColor() );
    if ( player->getPosition() == PlayerDown )
	move( ( g->getBrickCols()/2 - 1 ) * g->getBrickWidth(), ( BricksVertical - 1.5 ) * g->getBrickHeight());
    else //up
	move( ( g->getBrickCols()/2 - 1 ) * g->getBrickWidth(), 1 * g->getBrickHeight());
}

void Pad::setPosition( double x )
{
    double xpos = x * game->getBrickWidth();
    if ( xpos < 0 )
	xpos = 0;
    if ( xpos + boundingRect().width() > game->getCanvas()->width() ) 
	xpos = game->getCanvas()->width() - boundingRect().width();
    setX( xpos );
}

void Pad::advance( int phase )
{
    if ( phase == 1 ) 
	return;
    if ( x() == oldPosition )
	return;
    oldPosition = x();
    emit padPosition( (double)oldPosition / game->getBrickWidth() );
}

Brick::Brick( GameMain *g, int x, int y, const QColor& col, int score ) :
    QCanvasRectangle( x * g->getBrickWidth(), y * g->getBrickHeight(), 
		      g->getBrickWidth(), g->getBrickHeight(), g->getCanvas() ), game( g ),
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
    if ( !game->isRunning() )
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
    if ( game->isMyBall() )
	emit brickHit( boundingRect().center().x() / game->getBrickWidth(), boundingRect().center().x() / game->getBrickHeight() );
    delete this;
}

StandardBrick::StandardBrick( GameMain *game, int x, int y, const QColor& col, int score ) :
    Brick( game, x, y, col, score )
{
    ++bricksToDestroy;
}

StandardBrick::~StandardBrick()
{
    --bricksToDestroy;
}

StoneBrick::StoneBrick( GameMain *game, int x, int y, const QColor& col, int score ) :
    Brick( game, x, y, col, score )
{
    setBrush( QBrush( color, QCanvasRectangle::Dense5Pattern ) );
}

void StoneBrick::hit()
{
}

StoneBrick::~StoneBrick()
{
}

HardBrick::HardBrick( GameMain *game, int x, int y, const QColor& col, int hits, int score ) :
    Brick( game, x, y, col, score ), hitsLeft( hits )
{
    ++bricksToDestroy;
    paintIt();
}

void HardBrick::hit()
{
    if ( --hitsLeft <= 0 )
	Brick::hit();
    else {
	paintIt();
        if ( game->isMyBall() )
	    emit brickHit( boundingRect().center().x() / game->getBrickWidth(), boundingRect().center().x() / game->getBrickHeight() );
    }
}

void HardBrick::paintIt()
{
    switch ( hitsLeft ) {
	case 1: 
	    setBrush( QBrush( color, QCanvasRectangle::SolidPattern ) );
	    break;
	case 2:  
	    setBrush( QBrush( color, QCanvasRectangle::Dense1Pattern ) );
	    break;
	case 3: 
	    setBrush( QBrush( color, QCanvasRectangle::Dense2Pattern ) );
	    break;
	case 4: 
	    setBrush( QBrush( color, QCanvasRectangle::Dense3Pattern ) );
	    break;
	default: 
	    setBrush( QBrush( color, QCanvasRectangle::Dense4Pattern ) );
    }
    update();
}

HardBrick::~HardBrick()
{
    --bricksToDestroy;
}


DeathLine::DeathLine( GameMain *g, PlayerPosition position ) :
    QCanvasLine( g->getCanvas() ), game( g )
{
    setPen( QPen( QColor(), g->getBrickHeight() / 2, QCanvasLine::NoPen) );
    if ( position == PlayerDown )
        setPoints( 0, ( BricksVertical ) * g->getBrickHeight(), 
		   g->getBrickCols() * g->getBrickWidth() - 1, ( BricksVertical ) * g->getBrickHeight() );
    else
        setPoints( 0, 0, 
		   g->getBrickCols() * g->getBrickWidth() - 1, 0 );
}

void DeathLine::hit( Ball *ball )
{
    emit killedBall();
}

Lives::Lives( GameMain *g, Player *player, int lives ) :
    QCanvasText( g->getCanvas() ), game( g ), number( lives )
{
    QFont font;
    font.setBold( TRUE );
    font.setPixelSize( g->getBrickHeight() );
    setFont( font );
    setColor( game->getTextColor() );
    if ( player->getPosition() == PlayerDown )
	move( g->getBrickWidth(), ( BricksVertical - 1 ) * g->getBrickHeight() );
    else //up
	move( g->getBrickWidth(), 0 );
    print();
}

void Lives::looseLife()
{
    if ( number > 0 )
	--number;
    print();
    emit ballLost();
    emit livesChanged( number );
    if ( number <= 0) 
	emit noMoreLives();
    else {
	emit newBall();
    }
}

void Lives::setLives( int value )
{
    if ( number != value ) {
	number = value; 
	print();
	emit livesChanged( number );
    }
}

void Lives::print()
{
    setText( QString( "Lives: %1" ).arg( number ) );
    update();
}

Score::Score( GameMain *g, Player *player ) :
    QCanvasText( g->getCanvas() ), game( g ), number( 0 )
{
    QFont font;
    font.setBold( TRUE );
    font.setPixelSize( g->getBrickHeight() );
    setFont( font );
    setColor( game->getTextColor() );
    if ( player->getPosition() == PlayerDown )
        move( ( g->getBrickCols() - 5 ) * g->getBrickWidth(), ( BricksVertical - 1 ) * g->getBrickHeight() );
    else //up
        move( ( g->getBrickCols() - 5 ) * g->getBrickWidth(), 0 );
    print();
}
    
void Score::addScore( int value )
{
    number += value;
    if ( number < 0 )
	number = 0;
    print();
    emit scoreChanged( number );
}

void Score::print()
{
    setText( QString( "Score: %1" ).arg( number, 5 ) );
    update();
}

void Score::setScore( int value )
{
    if ( number != value ) {
	number = value; 
	print();
	emit scoreChanged( number );
    }
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
    //load();
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
    QString startTag;
    if ( game->isMultiplayer() )
	startTag = LevelMultiplayerTag;
    else
	startTag = levelStartTag;
    QTextStream t( &f );
    QStringList *lines;
    QString line;
    while ( !( line = t.readLine() ).isNull() ) {
	if ( line.contains( startTag, FALSE ) ) {
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

bool Level::start()
{
    bool ok = levels.first();
    if ( ok )
	createLevel();
    return ok;
}

bool Level::next()
{
    bool ok = levels.next();
    if ( ok )
	createLevel();
    return ok;
}

void Level::createLevel( QStringList *levelShape )
{
    QStringList *level;
    if ( levelShape )
	level = levelShape;
    else
	level = levels.current();
    if ( !level )
	return;
    QStringList invLevel;
    QString emptyLine;
    emptyLine.fill( '  ', game->getBrickCols() );
    for ( int i = game->getBrickRows() - 1; i >= 0; --i )
	invLevel.append( emptyLine );
    Brick *brick;
    char brickType;
    char brickColor;
    int row = 0;
    for ( QStringList::Iterator it = level->begin(); 
	    it != level->end() && row < game->getBrickRows() ; ++it, ++row ) {
	QString &line = (*it);
qDebug( "level: %s ", line.latin1() );
	for ( int col = 0; 
		col < QMIN( game->getBrickCols(), (int)line.length() / 2 ); ++col ) {
	    brickType = line[2 * col].upper();
	    brickColor = line[2 * col + 1].upper();
	    QString brickString = QString( QChar( brickType ) ) + QString( QChar( brickColor ) );
	    invLevel[ game->getBrickRows() - row - 1 ].replace( 2 * ( game->getBrickCols() - col - 1 ), 2, brickString );
	    QColor color;
	    switch( brickColor ) {
		case 'R': 
		    color = QColor( "red" );
		    break;
		case 'G': 
		    color = QColor( "green" );
		    break;
		case 'B': 
		    color = QColor( "blue" );
		    break;
		case 'C': 
		    color = QColor( "cyan" );
		    break;
		case 'M': 
		    color = QColor( "magenta" );
		    break;
		case 'Y': 
		    color = QColor( "yellow" );
		    break;
		case 'S': 
		    color = QColor( "gray" );
		    break;
	    }
	    if ( color.isValid() ) {
		brick = 0;
		switch( brickType ) {
		    case 'B': 
			brick = new StandardBrick( game, col, row, color, BrickPoints );
			break;
		    case '1': case '2': case '3': case '4': case '5': 
		    case '6': case '7': case '8': case '9':
		    {
			int hits = brickType - '0';
			brick = new HardBrick( game, col, row, color, hits, hits * BrickPoints );
			break;
		    }
		    case 'S': case '0': 
			brick = new StoneBrick( game, col, row, color, 10 * BrickPoints );
			break;
		}
		if ( brick ) {
		    //connect( brick, SIGNAL(score(int)), game->getMyScore(), SLOT(addScore(int)) );
		    connect( brick, SIGNAL(allDestroyed()), game, SLOT(endLevel()) );
		    if ( game->getRemote() )
			connect( brick, SIGNAL(brickHit(int,int)), game->getRemote(), SLOT(sendBrickHit(int,int)) );
		    brick->show();
		}
	    
	    } //if ( color.isValid() )
	} //for int col
    } //for QStringList::Iterator it

for ( int q=0; q < game->getBrickRows(); ++q )
qDebug( "invlv: %s ", invLevel[ q ].latin1() );

    if ( game->isMyBall() ) {
	emit levelCreated( &invLevel );
    }
}


Player::Player( GameMain *g, PlayerPosition pos ) :
    game( g ), position( pos )
{
    lives = new Lives( game, this, InitLives );
    lives->show();
    connect( lives, SIGNAL(ballLost()), game, SLOT(killBall()) );

    score = new Score( game, this );
    score->show();

    pad = new Pad( game, this );
    pad->show();
}

void Player::setMyBall()
{
    game->setCurrentPlayer( this );
}

TableView::TableView( GameMain *parent, const char *name, WFlags f ) :
    QCanvasView( parent->getCanvas(), parent, name, f ), game( parent )
{
    setFocusPolicy(QWidget::StrongFocus);
    //takeMouse( TRUE );
    setFocus();
}

void TableView::contentsMouseMoveEvent( QMouseEvent* me)
{
    int x = me->pos().x();
    Pad *pad = game->getMyPad();
    if ( x < 0 )
	x = 0;
    if ( x + pad->boundingRect().width() > game->getCanvas()->width() ) 
	x = game->getCanvas()->width() - pad->boundingRect().width();
    pad->setX( x );
    if ( game->isGluedBall() && game->isMyBall() && game->getBall()) {
	game->getBall()->putOnPad();
    }
}

void TableView::contentsMousePressEvent( QMouseEvent *me)
{
    if ( !mouseTaken ) {
	takeMouse( TRUE );
    } else if ( game->isGluedBall() ) {
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
    if ( !( QApplication::winVersion() & Qt::WV_NT_based ) )
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
    multiplayer = FALSE;
    remote = 0;
    textPaused = 0;

    QMenuBar *menu = menuBar();

    QPopupMenu *gameMenu = new QPopupMenu( menu );
    gameMenu->insertItem( "&New Game", this, SLOT(newGame()), CTRL+Key_N );
    gameMenu->insertSeparator();
    gameMenu->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
    menu->insertItem( "&Game", gameMenu );

    optionsMenu = new QPopupMenu( menu );
    paused_id = optionsMenu->insertItem( "&Pause", this, SLOT(togglePause()), Key_P );
    optionsMenu->setItemChecked(paused_id, FALSE);
    optionsMenu->insertSeparator();
    colors_id = optionsMenu->insertItem( "Inversed &Colors", this, SLOT(toggleColors()), CTRL+Key_C );
    optionsMenu->setItemChecked(colors_id, TRUE);
    menu->insertItem( "&Options", optionsMenu );
    
    QPopupMenu *multiplayerMenu = new QPopupMenu( menu );
    multiplayerMenu->insertItem( "&Serve multiplayer game", this, SLOT(serveMultiGame()) );
    multiplayerMenu->insertSeparator();
    multiplayerMenu->insertItem( "&Join multiplayer game", this, SLOT(joinMultiGame()) );
    menu->insertItem( "&Multiplayer", multiplayerMenu );
    
    QPopupMenu *helpMenu = new QPopupMenu( menu );
    helpMenu->insertItem( "&Help", this, SLOT(help()), Key_F1 );
    helpMenu->insertSeparator();
    helpMenu->insertItem( "&About Qt", this, SLOT(about()), CTRL+Key_A );
    menu->insertItem( "&Help", helpMenu );

    setFocusPolicy(QWidget::NoFocus);

    level = new Level( this, LevelFile, LevelBeginTag );

    newGame();
}

GameMain::~GameMain()
{
}

void GameMain::newGame( bool multi )
{
    if ( remote )
	multiplayer = multi;
    else
	multiplayer = FALSE;
    running = FALSE;
    QCanvasItemList cil = canvas->allItems();
    for (QCanvasItemList::Iterator it = cil.begin(); it != cil.end(); ++it) {
	if ( *it )
	    delete *it;
    }
    if ( !multiplayer )
	optionsMenu->setItemEnabled( paused_id, TRUE );
    else
	optionsMenu->setItemEnabled( paused_id, FALSE );
    player = new Player( this, PlayerDown );
    currentPlayer = player;
    if ( multiplayer ) {
	player2 = new Player( this, PlayerUp );
    
	connect( player->getScore(), SIGNAL(scoreChanged(int)), remote, SLOT(sendScore(int)) );
	connect( remote, SIGNAL(readScore(int)), player2->getScore(), SLOT(setScore(int)) );

	connect( player->getLives(), SIGNAL(livesChanged(int)), remote, SLOT(sendLives(int)) );
	connect( remote, SIGNAL(readLives(int)), player2->getLives(), SLOT(setLives(int)) );

	connect( player->getPad(), SIGNAL(padPosition(double)), remote, SLOT(sendPad(double)) );
	connect( remote, SIGNAL(readPad(double)), player2->getPad(), SLOT(setPosition(double)) );
	player->getPad()->setAnimated( TRUE );

	connect( this, SIGNAL(myBall()), remote, SLOT(sendMyBall()) );
	connect( remote, SIGNAL(itsBall()), this, SLOT(setItsBall()) );

	connect( remote, SIGNAL(brickHit(int,int)), this, SLOT(brickHit(int,int)) );

	connect( this, SIGNAL(start()), remote, SLOT(sendStart()) );
	connect( remote, SIGNAL(start()), this, SLOT(startBall()) );

	if ( !remote->isServer() ) {
	    currentPlayer = player;
	    connect( remote, SIGNAL(readLevel(QStringList*)), level, SLOT(createLevel(QStringList*)) );
	} else {
    	    connect( level, SIGNAL(levelCreated(QStringList*)), remote, SLOT(sendLevel(QStringList*)) );
	}
	//level multiplayer..
    }
    
    if ( !multiplayer || remote->isServer() ) {
	level->load();
	level->start();
    }

    deathLine = new DeathLine( this, player->getPosition() );
    deathLine->show();

    connect( deathLine, SIGNAL(killedBall()), player->getLives(), SLOT(looseLife()) );
    connect( player->getLives(), SIGNAL(newBall()), this, SLOT(startBall()) );
    connect( player->getLives(), SIGNAL(noMoreLives()), this, SLOT(endGame()) );
    
    tableView->takeMouse( TRUE );
    tableView->show();
    running = TRUE;

    ball = 0;
    startBall();
}


void GameMain::serveMultiGame()
{
    delete remote;
    remote = 0;
    tableView->takeMouse( FALSE );
    remote = NetworkDialog::makeConnection( TRUE, this );
    if ( remote ) {
	newGame( TRUE );
    }
}

void GameMain::joinMultiGame()
{
    delete remote;
    remote = 0;
    tableView->takeMouse( FALSE );
    remote = NetworkDialog::makeConnection( FALSE, this );
    if ( remote ) {
	newGame( TRUE );
    }
}


void GameMain::killBall()
{
    delete ball;
    ball = 0;
    setCurrentPlayer( player );
}

void GameMain::endGame()
{
    QFont font;
    font.setBold( TRUE );
    font.setPixelSize( brickHeight * 4 );
    QCanvasText *text = new QCanvasText( "GAME OVER", font, canvas );
    text->setColor( textColor );
    text->move( 2 * brickWidth, 5 * brickWidth );
    text->setZ( 255 );
    text->show();
}

void GameMain::endLevel()
{
    if ( remote && remote->isServer() ) {
	if ( !level->next() )
	    level->start();
    }
    startBall();
}


void GameMain::startBall()
{
    if ( !ball ) {
	ball = new Ball( this, currentPlayer );
	connect( ball, SIGNAL(ballOwnedBy(Player*)), this, SLOT(setCurrentPlayer(Player*)) );
	if ( remote ) {
	    connect( ball, SIGNAL(ballPosition(double,double,double,double)), remote, SLOT(sendBall(double,double,double,double)) );
	    connect( remote, SIGNAL(readBall(double,double,double,double)), ball, SLOT(setBall(double,double,double,double)) );
	}
    }
    setGluedBall( TRUE );
    ball->putOnPad();
    if ( currentPlayer->getPosition() == PlayerDown ) 
	ball->setSpeed( 0.25 * Pi, maxBallSpeed / 2 );
    else
	ball->setSpeed( 1.25 * Pi, maxBallSpeed / 2 );
    ball->show();
    if ( isMyBall() )
	emit start();
}

void GameMain::setCurrentPlayer( Player *pl )
{
    currentPlayer = pl;
    if ( currentPlayer == player )
	emit myBall();
}

void GameMain::setItsBall()
{
    currentPlayer = player2;
}

void GameMain::brickHit( int x, int y )
{
    QCanvasItemList cil = canvas->collisions( QPoint(x * brickWidth + brickWidth / 2, y * brickHeight + brickHeight / 2 ) );
    for ( QCanvasItemList::Iterator it = cil.begin(); it != cil.end(); ++it) {
	QCanvasItem *item = *it;
	if ( item->rtti() == Brick::RTTI ) {
	    ( (Brick*)item )->hit();
	    break;
	}
    }
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
	    case Ball::RTTI: 
		( (Ball*)item )->setBrush( ballColor );
		break;
	    case Pad::RTTI: 
		( (Pad*)item )->setBrush( padColor );
		break;
	    case Lives::RTTI: 
		( (Lives*)item )->setColor( textColor );
		break;
	    case Score::RTTI: 
		( (Score*)item )->setColor( textColor );
		break;
	    default: 
		if( item->rtti() == QCanvasText::RTTI) {
		    ( (QCanvasText*)item )->setColor( textColor );
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

void GameMain::togglePause()
{
    bool paused = !optionsMenu->isItemChecked( paused_id );
    optionsMenu->setItemChecked( paused_id, paused );
    if ( paused ) {
	QFont font;
	font.setBold( TRUE );
	font.setPixelSize( brickHeight * 4 );
	textPaused = new QCanvasText( "PAUSED", font, canvas );
	textPaused->setColor( textColor );
	textPaused->move( 5 * brickWidth, 5 * brickWidth );
	textPaused->setZ( 255 );
	textPaused->show();
	running = FALSE;
        canvas->setAdvancePeriod( -1 );
    } else {
	delete textPaused;
	textPaused = 0;
	running = TRUE;
        canvas->setAdvancePeriod( advancePeriod );
    }
    canvas->update();
}

void GameMain::setGluedBall( bool glued )
{
    gluedBall = glued;
    if ( ball ) {
	if ( glued )
	    ball->stop();
	else
	    ball->start();
    }
}


void GameMain::keyPressEvent ( QKeyEvent *e )
{
    tableView->setFocus();
}


RemoteServer::RemoteServer( Q_UINT16 port, QObject *parent, const char *name ) :
    QServerSocket( port, 1, parent, name )
{
}

void RemoteServer::newConnection( int socket )
{
    RemoteSocket *remoteSocket = new RemoteSocket( socket, this );
    emit newRemoteSocket( remoteSocket );
}


RemoteSocket::RemoteSocket( int socketConnection, QObject *parent, const char *name ) :
    QSocket( parent, name )
{
    setSocket( socketConnection );
}   
    
RemoteSocket::RemoteSocket( const QString &host, Q_UINT16 port, QObject *parent, const char *name ) :
    QSocket( parent, name )
    
{
    connectToHost( host, port );
}

Remote::Remote( GameMain *g, Q_UINT16 port ) :
    QObject( g ), game( g )
{
    remoteSocket = 0;
    iAmServer = TRUE;
    remoteServer = new RemoteServer( port, this );
    connect( remoteServer, SIGNAL(newRemoteSocket(RemoteSocket*)), this, SLOT(remoteSocketCreated(RemoteSocket*)) );
}

Remote::Remote( GameMain *g, const QString &host, Q_UINT16 port ) :
    QObject( g ), game( g )
{
    remoteServer = 0;
    iAmServer = FALSE;
    remoteSocket = new RemoteSocket( host, port, this );
    init();
}


void Remote::remoteSocketCreated( RemoteSocket *remoteSock )
{
    remoteSocket = remoteSock;
    init();
    emit success();
}
   
void Remote::init()
{
    connect( remoteSocket, SIGNAL(connected()), this, SLOT(socketConnected()) );
    connect( remoteSocket, SIGNAL(connectionClosed()), this,  SLOT(socketConnectionClosed()) );
    connect( remoteSocket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()) );
    connect( remoteSocket, SIGNAL(error(int)), this, SLOT(socketError(int)) );
    stream.setDevice( remoteSocket );
}

void Remote::socketConnected()
{
    emit success();
}

void Remote::socketConnectionClosed()
{
    qDebug( "Connection closed!" );
}

void Remote::socketError( int e )
{
    emit failed();
}

void Remote::socketReadyRead()
{
    QString line;
    while ( remoteSocket->canReadLine() ) {
	line = remoteSocket->readLine();  
qDebug( "Received: %s", line.latin1() );
	if ( line.section( ',', 0, 0 ) == "score" )
	    emit readScore( line.section( ',', 1, 1 ).toInt() );
	else if ( line.section( ',', 0, 0 ) == "lives" )
	    emit readLives( line.section( ',', 1, 1 ).toInt() );
	else if ( line.section( ',', 0, 0 ) == "pad" )
	    emit readPad( game->getBrickCols() - line.section( ',', 1, 1 ).toDouble() - 2 ); 
	    // 2 is pad width
	else if ( line.section( ',', 0, 0 ) == "ball" )
	    emit readBall( game->getBrickCols() - line.section( ',', 1, 1 ).toDouble(),
	                   game->getBrickRows() - line.section( ',', 2, 2 ).toDouble(), 
	                   Pi + line.section( ',', 3, 3 ).toDouble(), 
			   line.section( ',', 4, 4 ).toDouble() );
	else if ( line.section( ',', 0, 0 ) == "myball" )
	    emit itsBall();
	else if ( line.section( ',', 0, 0 ) == "brick" )
	    emit brickHit( game->getBrickCols() - line.section( ',', 1, 1 ).toInt() - 1,
			   game->getBrickRows() - line.section( ',', 2, 2 ).toInt() - 1);
	else if ( line.section( ',', 0, 0 ) == "start" )
	    emit start();
	else if ( line.startsWith( "level" ) ) {
	    QStringList level;
	    while ( remoteSocket->canReadLine() ) {
		line = remoteSocket->readLine();  
qDebug( "Received: %s", line.latin1() );
		level.append( line );
		if ( line.contains( LevelEndTag, FALSE ) )
		    break;
	    }
	    emit readLevel( &level );
	}
    }
}

void Remote::send( const QString &line )
{
    stream << line << "\n";
qDebug( "Sent: %s", line.latin1() );
}

void Remote::sendScore( int score )
{
    send( QString( "score,%1" ).arg( score ) );
}

void Remote::sendLives( int lives )
{
    send( QString( "lives,%1" ).arg( lives ) );
}

void Remote::sendPad( double xpos )
{
    send( QString( "pad,%1" ).arg( xpos ) );
}

void Remote::sendBall( double x, double y, double ang, double sp )
{
    send( QString( "ball,%1,%2,%3,%4" ).arg( x ).arg( y ).arg( ang ).arg( sp ) );
}

void Remote::sendMyBall()
{
    send( "myball" );
}

void Remote::sendBrickHit( int x, int y )
{
    send( QString( "brick,%1,%2" ).arg( x ).arg( y ) );
}

void Remote::sendStart()
{
    send( "start" );
}

void Remote::sendLevel( QStringList *levelShape )
{
    send( "level" );
    send( LevelMultiplayerTag );
    for ( QStringList::Iterator it = levelShape->begin(); it != levelShape->end(); ++it ) {
	send( *it );
    }
    send( LevelEndTag );
}



NetworkDialog::NetworkDialog( bool srv, GameMain *parent, const char *name ) :
    QDialog( parent, name, TRUE ), server( srv ), remote( 0 ), owner( parent )
{
    QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );
    if ( server ) {
        setCaption( "Serve the game");
    } else {
	setCaption( "Join the game");
	QHBoxLayout *hbox0 = new QHBoxLayout( 6 );
	vbox->addLayout( hbox0 );
	QLabel* l1 = new QLabel( "Host:", this );
	hbox0->addWidget( l1 );
	editHost = new QLineEdit( this );
	hbox0->addWidget( editHost );
    }

    QHBoxLayout *hbox1 = new QHBoxLayout( 6 );
    vbox->addLayout( hbox1 );
    QLabel* l2 = new QLabel( "Port:", this );
    hbox1->addWidget( l2 );
    editPort = new QLineEdit( QString::number( NetworkPort ), this );
    hbox1->addWidget( editPort );

    QHBoxLayout *hbox2 = new QHBoxLayout( 6 );
    vbox->addLayout( hbox2 );
    ok = new QPushButton( "OK", this );
    ok->setDefault( TRUE );
    QPushButton *cancel = new QPushButton( "Cancel", this );
    QSize bs( ok->sizeHint() );
    if ( cancel->sizeHint().width() > bs.width() )
	bs.setWidth( cancel->sizeHint().width() );
    ok->setFixedSize( bs );
    cancel->setFixedSize( bs );
    hbox2->addWidget( ok );
    hbox2->addWidget( cancel );
    resize( QMAX( sizeHint().width(), 100 ), sizeHint().height() );

    connect( ok, SIGNAL( clicked() ), this, SLOT( tryConnect() ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void NetworkDialog::tryConnect()
{
    ok->hide();
    if ( server ) {
	remote = new Remote( owner, editPort->text().toInt() );
    } else {
	remote = new Remote( owner, editHost->text(), editPort->text().toInt() );
    }
    connect( remote, SIGNAL(success()), this, SLOT(accept()) );
    connect( remote, SIGNAL(failed()), this, SLOT(failed()) );
}

void NetworkDialog::failed()
{
    QMessageBox::critical( this, "Error", "Connection error!", QMessageBox::Ok, 0 );
    delete remote;
    remote = 0;
    ok->show();
}

Remote *NetworkDialog::makeConnection( bool srv, GameMain *parent, const char *name )
{
    NetworkDialog dlg( srv, parent, name );
    bool ok = ( dlg.exec() == QDialog::Accepted );
    if ( ok )
	return dlg.remote;
    else
	return 0;
}
