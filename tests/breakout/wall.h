#ifndef WALL_H
#define WALL_H

#include <qcanvas.h>
#include <qmainwindow.h>

const double Pi = 3.1415926535;
const int BricksHorizontal = 20;
const int BricksVertical = 20;
const int InitBrickWidth = 30;
const int InitBrickHeight = 20;
const int InitLives = 3;
const int BrickPoints = 10;
const char LevelFile[] = "levels.dat";
const char LevelBeginTag[] = "START LEVEL";
const char LevelEndTag[] = "END LEVEL";

class GameMain;

class Ball : public QObject, public QCanvasEllipse 
{
    Q_OBJECT
public:
    enum { RTTI = 7731 };
    Ball( GameMain *game );
    void advance( int phase );
    int rtti() const { return RTTI; }
public slots:
    void setSpeed( double ang, double sp );
private:
    GameMain *game;
    double angle;
    double speed;
};

class Pad : public QObject, public QCanvasRectangle 
{
    Q_OBJECT
public:
    enum { RTTI = 7732 };
    Pad( GameMain *game );
    int rtti() const { return RTTI; }
private:
    GameMain *game;
};

class Brick : public QObject, public QCanvasRectangle
{
    Q_OBJECT
public:
    enum { RTTI = 7733 };
    Brick( GameMain *game, int x, int y, const QColor& col = QColor( "red" ), int score = 0 );
    ~Brick();
    int rtti() const { return RTTI; }
    virtual void hit();
signals:
    void score( int value );
    void allDestroyed();
protected:
    GameMain *game;
    int points;
    QColor color;
    static int bricksToDestroy;
};

class StandardBrick : public Brick
{
public:
    StandardBrick( GameMain *game, int x, int y, const QColor& col, int score = 0 );
};

class StoneBrick : public Brick
{
public:
    StoneBrick( GameMain *game, int x, int y, const QColor& col = QColor( "gray" ), int score = 0 );
    void hit();
};

class HardBrick : public Brick
{
public:
    HardBrick( GameMain *game, int x, int y, const QColor& col, int hits = 1, int score = 0 );
    void hit();
protected:
    int hitsLeft;
    void paintIt();
};


class DeathLine : public QObject, public QCanvasLine
{
    Q_OBJECT
public:
    enum { RTTI = 7734 };
    DeathLine( GameMain *game );
    int rtti() const { return RTTI; }
    void hit( Ball *ball );
signals:
    void killedBall();
private:
    GameMain *game;
};

class Lives : public QObject, public QCanvasText
{
    Q_OBJECT
public:
    enum { RTTI = 7735 };
    Lives( GameMain *g, int lives = InitLives );
    int rtti() const { return RTTI; }
    int getLives() const { return number; }
    void setLives( int value ) { number = value; }
public slots:
    void looseLife();
signals:
    void newBall();
    void noMoreLives();
private:
    GameMain *game;
    void print();
    int number;
};

class Score : public QObject, public QCanvasText
{
    Q_OBJECT
public:
    enum { RTTI = 7736 };
    Score( GameMain *g );
    int rtti() const { return RTTI; }
    int getScore() const { return number; }
public slots:
    void addScore( int value );
    void setScore( int value ) { number = value; }
private:
    GameMain *game;
    void print();
    int number;
};


class TempItems : public QObject
{
    Q_OBJECT
public:
    TempItems( int ms, QCanvas *c );
    TempItems( QCanvasItem *item, int ms );
    void addItem( QCanvasItem *item );
    void start();
public slots:
    void stop();
private:
    QCanvasItemList list;
    QCanvas *canvas;
    int time;
};

//------------------------------------------------



class Level : public QObject
{
    Q_OBJECT
public:
    Level( GameMain *g, const QString &filename, const QString &startTag );
    void load( const QString &filename = 0 );
    void start();
    void next();
signals:
    void noMoreLevels();
private:
    void createLevel();
    GameMain *game;
    QString levelFile;
    QString levelStartTag;
    QPtrList< QStringList > levels;
};


/*class Player : public QObject
{
    Q_OBJECT
public:
    Player();
protected:
    Pad *pad;
    Lives *lives;
    Score *score;
};*/

/*class Network : public QObject
{
    Q_OBJECT
public:

public slots:
    void sendBallSpeed( double ang, double sp );
    void sendLives( int lives );
    void sendScore( int score );
    void sendDied();
signals:
    void readBallSpeed( double ang, double sp );
    void readLives( int lives );
    void readScore( int score );
    void readDied();

private:

}*/

class TableView : public QCanvasView 
{
public:
    TableView( GameMain *parent = 0, const char *name = 0, WFlags f = 0 );
protected:
    void contentsMouseMoveEvent( QMouseEvent* me);
    void contentsMousePressEvent( QMouseEvent* me);
    void keyPressEvent( QKeyEvent* e );
    void takeMouse( bool take );
    void focusOutEvent( QFocusEvent * e );
private:
    GameMain *game;
    bool mouseTaken;
};


class GameMain : public QMainWindow 
{
    Q_OBJECT
public:
    GameMain( int cols = BricksHorizontal, int rows = BricksVertical, int width = InitBrickWidth, int height = InitBrickHeight, QWidget *parent = 0, const char *name = 0, WFlags f = WType_TopLevel );
    ~GameMain();
    Pad* getPad() const { return pad; }
    TableView* getTableView() const { return tableView; }
    QCanvas* getCanvas() const { return canvas; }
    Score *getScore() const { return score; }
    Ball *getBall() const { return ball; }
    int getBrickWidth() const { return brickWidth; }
    int getBrickHeight() const { return brickHeight; }
    int getBrickCols() const { return brickCols; }
    int getBrickRows() const { return brickHeight; }
    double getMaxBallSpeed() const { return maxBallSpeed; }
    bool getRunning() const { return running; }
    bool getGluedBall() const { return getGluedBall; }
    void setGluedBall( bool glued );
    const QColor& getBallColor() const { return ballColor; }
    const QColor& getPadColor() const { return padColor; }
    const QColor& getTextColor() const { return textColor; }
    void setInverseColors( bool inver );
public slots:
    void endGame();
    void endLevel();
    void startBall();
    void newGame();
    void help();
    void about();
    void toggleColors();
signals:
    void lostLife();
protected:
    void keyPressEvent( QKeyEvent* e );
    
private:
    TableView *tableView;
    QCanvas *canvas;
    QPopupMenu *optionsMenu;
    int colors_id;

    int brickWidth;
    int brickHeight;
    int brickCols;
    int brickRows;
    bool inverseColors;
    QColor ballColor;
    QColor padColor;
    QColor textColor;
    int advancePeriod;
    double maxBallSpeed;
    bool running;
    bool gluedBall;

    Ball *ball;
    Pad *pad;
    Lives *lives;
    Score *score;
    DeathLine *deathLine;
    Level *level;
};


#endif