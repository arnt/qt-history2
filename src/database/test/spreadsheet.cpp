#include "spreadsheet.h"

// return evaluated token
// token can be a cell location, numerical constant or mathematical operator
QString EquationCell::evalToken( QString& tkn ) const
{
    if ( tkn.length() && tkn[0].isLetter() ) {
	int c = QChar(tkn[0]) - 'A';
	bool ok;
	int r = tkn.mid( 1, tkn.length() ).toInt(&ok);
	if ( ok )
	    r--;
	if ( c >= 0 && ok && r >= 0 ) {
	    if ( r == row() && c == col() ) // avoid infinit loop
		return "ERROR";
	    QTableItem* sc = table()->item( r, c );
	    if ( sc )
		return sc->text();
	}
    }
    return tkn;
}

// return relative precedence of operator c
int EquationCell::precedence( QChar c ) const
{
    if ( c == '+' )
	return 1;
    if ( c == '-' )
	return 1;
    if ( c == '*' )
	return 2;
    if ( c == '/' )
	return 2;
    return 0;
}

// return infix expression s as a postfix expression
// operators and terms are separated by spaces
QString EquationCell::toPostfix( QString s ) const
{
    s = s.replace( QRegExp("\\s"), "" );
    CharStack stack;
    QString currToken;
    QString postfixString;
    for ( uint i = 0; i < s.length(); ++i ) {
	QChar c = s[(int)i];
	if ( c.isLetter() || c.isNumber() )
	    postfixString += c ;
	else {
	    if ( c == '(' )
		stack.push( c );
	    if ( c == ')' && stack.count() ) {
		QChar m = stack.pop();
		while ( stack.count() && m  != '(' ){
		    postfixString += " ";
		    postfixString += m;
		    m = stack.pop();
		}
	    }
	    if ( c == '*' || c == '/' || c == '+' || c == '-' ){
		while ( stack.count() && ( precedence(stack.top()) >= precedence(c) ) ) {
		    QChar m = stack.pop();
		    postfixString += " ";
		    postfixString += m;
		}
		stack.push( c );
		postfixString += " ";
	    }
	}
    }
    while ( stack.count() ) {
	postfixString += " ";
	postfixString += stack.pop();
    }
    return postfixString;
}

// s is a legal postfix expression with tokens separated by spaces
// return result of evaluation as a string
QString EquationCell::eval( QString s ) const
{
    int start = 0;
    int end = 0;
    DoubleStack stack;
    while ( TRUE ) {
	end = s.find( QRegExp("[\\s+\\-*/]"), start );
	if ( end != -1 ) {
	    QString term;
	    if ( end == start ) {
		term = QString(s[start]);
		start = end + 2;
	    } else {
		term = s.mid( start, end-start );
		start = end + 1;
	    }
	    bool ok;
	    double val = evalToken( term ).toDouble(&ok);
	    if ( !ok ) {
		if ( term.find( QRegExp("[+\\-*/]")) != -1 ) {
		    double d2 = stack.pop();
		    double d1 = stack.pop();
		    double d3 = 0.0;
		    if ( term == "+" )
			d3 = d1 + d2;
		    if ( term == "-" )
			d3 = d1 - d2;
		    if ( term == "*" )
			d3 = d1 * d2;
		    if ( term == "/" )
			d3 = d1 / d2;
		    stack.push( d3 );
		} else {
		    return term + QString(":ERROR");
		}
	    } else {
		stack.push( val );
	    }
	} else {
	    break;
	}
    }
    if ( stack.count() )
	return QString::number(stack.pop());
    else
	return s;
}

void EquationCell::paint ( QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected )
{
    setText(eval(toPostfix(eq)));
    QColorGroup mycg(cg);
    mycg.setColor(QColorGroup::Text, Qt::red);
    mycg.setColor(QColorGroup::HighlightedText, cyan);
    QTableItem::paint(p,mycg,cr, selected);
}

///////////

Spreadsheet::Spreadsheet ( QWidget * parent, const char * name )
    : QTable(parent,name)
{
    setNumRows(100);
    setNumCols(26);
    QString l("%1");
    for ( int i = 0; i < 26; ++i )
	horizontalHeader()->setLabel( i, QChar('A'+i) );
    setMinimumWidth(400);
    setMinimumHeight(400);
    connect( this, SIGNAL(currentChanged(int,int)),
	     SLOT(newSelection(int,int)));
}

void Spreadsheet::clearCurrent()
{
    clearCell(currentRow(),currentColumn());
}

void Spreadsheet::editEquation()
{
    int row = currentRow();
    int col = currentColumn();
    QString def;
    QTableItem* sc = item( row, col);
    if ( sc ) {
	if ( sc->editType() == QTableItem::Never )
	    def = ((EquationCell*)sc)->equation();
	else
	    def = sc->text();
    }
    bool ok = FALSE;
    QString eq = QInputDialog::getText( tr( "Equation" ),
					tr( "Enter Equation:" ),
					QLineEdit::Normal,
					def,
					&ok, this, "Equation Editor" );
    if ( ok && !eq.isEmpty() )
	setItem( row, col, new EquationCell( eq.upper(), this ) );
}

void Spreadsheet::newSelection( int row, int col )
{
    QTableItem* i = item( row, col );
    if ( i && i->editType() == QTableItem::Never )
	emit newTextSelected( ((EquationCell*)i)->equation() );
    else if ( i )
	emit newTextSelected( i->text() );
    else
	emit newTextSelected( " " );
}
