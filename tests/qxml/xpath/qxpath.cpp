#include "qlist.h"
#include "qxpath.h"

/***************************************************************
 *
 * QXPathNS: "name space" for the XPath classes
 *
 ***************************************************************/
class QXPathNS {
public:
    enum Token {
	TkError,	// an error occured during tokenizing
	TkLeftParen,	// '('
	TkRightParen,	// ')'
	TkLeftBracket,	// '['
	TkRightBracket,	// ']'
	TkSelfAbbr,	// '.'
	TkParentAbbr,	// '..'
	TkAttribAbbr,	// '@'
	TkComma,	// ','
	TkDoubleColon,	// '::'
	TkNameTest_Star,
	TkNameTest_NCNameStar,
	TkNameTest_QName,
	TkNodeType_Comment,
	TkNodeType_Text,
	TkNodeType_PI,
	TkNodeType_Node,
	TkOperator,
	TkFunctionName,
	TkAxisName,
	TkLiteral,
	TkNumber,
	TkVariableReference
    };

    enum Operator {
	OpError,
	OpAnd,		// 'and'
	OpOr,		// 'or'
	OpMod,		// 'mod'
	OpDiv,		// 'div'
	OpSlash,	// '/'
	OpDoubleSlash,	// '//'
	OpPipe,		// '|'
	OpMultiply,	// '*'
	OpPlus,		// '+'
	OpMinus,	// '-'
	OpEqual,	// '='
	OpNotEqual,	// '!='
	OpLt,		// '<'
	OpLtEq,		// '<='
	OpGt,		// '>'
	OpGtEq		// '>='
    };

    enum Axis {
	AxError,
	AxChild,
	AxDescendant,
	AxParent,
	AxAncestor,
	AxFollowingSibling,
	AxPrecedingSibling,
	AxFollowing,
	AxPreceding,
	AxAttribute,
	AxNamespace,
	AxSelf,
	AxDescendantOrSelf,
	AxAncestorOrSelf
    };
};


/***************************************************************
 *
 * QXPathAtom (atom expressions for the expression tree)
 *
 ***************************************************************/
class QXPathAtom
{
public:
    enum Type {
	LocationPath,
	LocationStep,
	VariableReference,
	Literal,
	Number,
	Operator,
	FunctionCall
    };

    QXPathAtom()
    {
	parent = 0;
    }
    virtual ~QXPathAtom()
    {}

    virtual bool evaluate( QXPathDataType* ret ) = 0;
    virtual Type type() = 0;

    QXPathAtom *parent;
};


// Location Path node
class QXPathAtomLocationPath : public QXPathAtom
{
public:
    QXPathAtomLocationPath() : QXPathAtom()
    {
	children.setAutoDelete( TRUE );
    }
    ~QXPathAtomLocationPath()
    {
	children.clear();
    }

    void addChild( QXPathAtom* child )
    {
	children.append( child );
	child->parent = this;
    }

    bool evaluate( QXPathDataType* )
    {
	QXPathAtom *it;
	for ( it=children.first(); it!=0; it=children.next() ) {
	    QXPathDataType *t;
	    it->evaluate( t );
	}
	return TRUE;
    }

    Type type()
    {
	return LocationPath;
    }

private:
    QList<QXPathAtom> children; // QList is more complex than needed
};

// Location Step node
class QXPathAtomLocationStep : public QXPathAtom
{
public:
    QXPathAtomLocationStep( QXPathNS::Axis axis, const QString &nodeTest, QXPathAtom *predicate ) : QXPathAtom()
    {
	valueAxis = axis;
	valueNodeTest = nodeTest;
	valuePredicate = predicate;
    }
    ~QXPathAtomLocationStep()
    {
	delete valuePredicate;
    }

    bool evaluate( QXPathDataType* )
    {
	return TRUE;
    }

    Type type()
    {
	return LocationStep;
    }

private:
    QXPathNS::Axis  valueAxis;
    QString	    valueNodeTest; // ### use something else?
    QXPathAtom	    *valuePredicate;
};

// Variable Reference node
class QXPathAtomVariableReference : public QXPathAtom
{
public:
    QXPathAtomVariableReference( const QString &name ) : QXPathAtom()
    {
	value = name;
    }
    ~QXPathAtomVariableReference()
    {}

    bool evaluate( QXPathDataType* )
    {
	return TRUE;
    }

    Type type()
    {
	return VariableReference;
    }

private:
    QString value;
};

// String Literal node
class QXPathAtomLiteral : public QXPathAtom
{
public:
    QXPathAtomLiteral( const QString string ) : QXPathAtom()
    {
	value = string;
    }
    ~QXPathAtomLiteral()
    {}

    bool evaluate( QXPathDataType* )
    {
	return TRUE;
    }

    Type type()
    {
	return Literal;
    }

private:
    QString value;
};

// Number node
class QXPathAtomNumber : public QXPathAtom
{
public:
    QXPathAtomNumber( double number ) : QXPathAtom()
    {
	value = number;
    }
    ~QXPathAtomNumber()
    {}

    bool evaluate( QXPathDataType* )
    {
	return TRUE;
    }

    Type type()
    {
	return Number;
    }

private:
    double value;
};

// Operator node
class QXPathAtomOperator : public QXPathAtom
{
public:
    QXPathAtomOperator( QXPathNS::Operator op ) : QXPathAtom()
    {
	value = op;
	children.setAutoDelete( TRUE );
    }
    ~QXPathAtomOperator()
    {
	children.clear();
    }

    void addChild( QXPathAtom* child )
    {
	children.append( child );
	child->parent = this;
    }

    bool evaluate( QXPathDataType* )
    {
	QXPathAtom *it;
	for ( it=children.first(); it!=0; it=children.next() ) {
	    QXPathDataType *t;
	    it->evaluate( t );
	}
	return TRUE;
    }

    Type type()
    {
	return Operator;
    }

private:
    QXPathNS::Operator value;
    QList<QXPathAtom> children; // QList is more complex than needed
};

// Function Call node
class QXPathAtomFunctionCall : public QXPathAtom
{
public:
    QXPathAtomFunctionCall( const QString &name ) : QXPathAtom()
    {
	value = name;
	children.setAutoDelete( TRUE );
    }
    ~QXPathAtomFunctionCall()
    {
	children.clear();
    }

    void addChild( QXPathAtom* child )
    {
	children.append( child );
	child->parent = this;
    }

    bool evaluate( QXPathDataType* )
    {
	QXPathAtom *it;
	for ( it=children.first(); it!=0; it=children.next() ) {
	    QXPathDataType *t;
	    it->evaluate( t );
	}
	return TRUE;
    }

    Type type()
    {
	return FunctionCall;
    }

private:
    QString value;
    QList<QXPathAtom> children; // QList is more complex than needed
};


/***************************************************************
 *
 * QXPathLexicalAnalyzer
 *
 ***************************************************************/
class QXPathLexicalAnalyzer : public QXPathNS
{
public:
    QXPathLexicalAnalyzer( const QString &e );
    ~QXPathLexicalAnalyzer();

    Token nextToken();

    QString getString();
    double getNumber() const;
    Operator getOperator() const;
    Axis getAxis() const;

    bool atEnd() const;

private:
    void eatWs();
    QChar lookAhead( int offset ) const;

    QString expr;
    uint parsePos;
    Token token;

    QString str;
    Operator op;
    Axis axis;
};


/*!
  Constructor.
*/
QXPathLexicalAnalyzer::QXPathLexicalAnalyzer( const QString &e ) :
    expr(e), parsePos(0), token(TkError)
{
}

/*!
  Destructor.
*/
QXPathLexicalAnalyzer::~QXPathLexicalAnalyzer()
{
}

/*!
  Tokens are taken from the XPath 1.0 recommendation, section 3.7
  Returns the next token.
*/
QXPathNS::Token QXPathLexicalAnalyzer::nextToken()
{
    eatWs();
    if ( atEnd() ) {
	token = TkError;
	return token;
    }

    bool forceVariableReference = FALSE;
    QChar parseChar = expr[parsePos];

    // look for numbers
    // Attention: This test must occur before the test for a single '.' in
    // the "special stuff" section.
    if ( parseChar.isNumber() ||
	    (parseChar == '.' && lookAhead(1).isNumber()) ) {
	uint strBegin = parsePos;
	parsePos++;
	while ( !atEnd() ) {
	    parseChar = expr[parsePos];
	    if ( parseChar.isNumber() || parseChar == '.' )
		parsePos++;
	    else
		break;
	}
	str = expr.mid( strBegin, parsePos-strBegin );
	token = TkNumber;
	return token;
    }

    // look for the special stuff
    if        ( parseChar == '(' ) {
	token = TkLeftParen;
	goto finished;
    } else if ( parseChar == ')' ) {
	token = TkRightParen;
	goto finished;
    } else if ( parseChar == '[' ) {
	token = TkLeftBracket;
	goto finished;
    } else if ( parseChar == ']' ) {
	token = TkRightBracket;
	goto finished;
    } else if ( parseChar == '.' ) {
	if ( lookAhead(1) == '.' ) {
	    parsePos++;
	    token = TkParentAbbr;
	} else {
	    token = TkSelfAbbr;
	}
	goto finished;
    } else if ( parseChar == '@' ) {
	token = TkAttribAbbr;
	goto finished;
    } else if ( parseChar == ',' ) {
	token = TkComma;
	goto finished;
    } else if ( parseChar == ':' ) {
	if ( lookAhead(1) == ':' ) {
	    parsePos++;
	    token = TkDoubleColon;
	    goto finished;
	}
    } else if ( parseChar == '*' ) {
	switch ( token ) {
	    case TkAttribAbbr:
	    case TkDoubleColon:
	    case TkLeftParen:
	    case TkRightParen:
	    case TkOperator:
		token = TkNameTest_Star;
		break;
	    default:
		token = TkOperator;
		op = OpMultiply;
		break;
	}
	goto finished;
    } else if ( parseChar == '"' || parseChar == '\'' ) {
	parsePos++;
	uint strBegin = parsePos;
	while ( !atEnd() ) {
	    if ( expr[parsePos] == parseChar ) {
		token = TkLiteral;
		str = expr.mid( strBegin, parsePos-strBegin );
		goto finished;
	    }
	    parsePos++;
	}
	// we reached end without finding closing " or '
	token = TkError;
	goto finished;
    }

    // look for the "non-special" operators ('<', '<=', etc.)
    if ( parseChar == '=' ) {
	token = TkOperator;
	op = OpEqual;
	goto finished;
    } else if ( parseChar == '/' ) {
	token = TkOperator;
	if ( lookAhead(1) == '/' ) {
	    parsePos++;
	    op = OpDoubleSlash;
	} else {
	    op = OpSlash;
	}
	goto finished;
    } else if ( parseChar == '|' ) {
	token = TkOperator;
	op = OpPipe;
	goto finished;
    } else if ( parseChar == '+' ) {
	token = TkOperator;
	op = OpPlus;
	goto finished;
    } else if ( parseChar == '-' ) {
	token = TkOperator;
	op = OpMinus;
	goto finished;
    } else if ( parseChar == '!' && lookAhead(1) == '=' ) {
	token = TkOperator;
	parsePos++;
	op = OpNotEqual;
	goto finished;
    } else if ( parseChar == '<' ) {
	token = TkOperator;
	if ( lookAhead(1) == '=' ) {
	    parsePos++;
	    op = OpLtEq;
	} else {
	    op = OpLt;
	}
	goto finished;
    } else if ( parseChar == '>' ) {
	token = TkOperator;
	if ( lookAhead(1) == '=' ) {
	    parsePos++;
	    op = OpGtEq;
	} else {
	    op = OpGt;
	}
	goto finished;
    }

    // look for NCName and related stuff (QName, functions, axis, etc.)
    if ( parseChar == '$' ) {
	forceVariableReference = TRUE;
	parsePos++;
    }
    if ( parseChar.isLetter() || parseChar == '_' ) {
	uint strBegin = parsePos;
	bool forceQName = FALSE;
	parsePos++;
	while( !atEnd() ) {
	    QChar tmp = expr[parsePos];
	    if ( tmp.isLetter() || tmp.isDigit()
		    || tmp == '.' || tmp == '-' || tmp == '_' ) {
		// ### combining char and extender are missing
		parsePos++;
	    } else if ( tmp == ':' && lookAhead(1) != ':' && lookAhead(1) != '*' ) {
		// QName; in this case NCNames are not allowed anymore
		parsePos++;
		forceQName = TRUE;
	    } else {
		break;
	    }
	}
	str = expr.mid( strBegin, parsePos-strBegin );
	eatWs();
	parseChar = expr[parsePos]; // get char at actual position

	// variableReference overwrites all other tests
	if ( forceVariableReference ) {
	    token = TkVariableReference;
	    goto finished;
	}
	// must an operator be recognized?
	switch ( token ) {
	    case TkAttribAbbr:
	    case TkDoubleColon:
	    case TkLeftParen:
	    case TkRightParen:
	    case TkOperator:
		token = TkOperator;
		if ( str == "and" ) {
		    op = OpAnd;
		} else if ( str == "or" ) {
		    op = OpOr;
		} else if ( str == "mod" ) {
		    op = OpMod;
		} else if ( str == "div" ) {
		    op = OpDiv;
		} else {
		    token = TkError;
		}
		return token;
	    default:
		break;
	}

	// no operator
	if ( forceQName ) {
	    token = TkNameTest_QName;
	    goto finished;
	}
	if ( parseChar == '(' ) {
	    if ( str == "comment" ) {
		token = TkNodeType_Comment;
	    } else if ( str == "text" ) {
		token = TkNodeType_Text;
	    } else if ( str == "processing-instruction" ) {
		token = TkNodeType_PI;
	    } else if ( str == "node" ) {
		token = TkNodeType_Node;
	    } else {
		token = TkFunctionName;
	    }
	} else if ( parseChar == ':' ) {
	    if ( lookAhead(1) == ':' ) {
		if ( str == "child" ) {
		    token = TkAxisName;
		    axis = AxChild;
		} else if ( str == "descendant" ) {
		    token = TkAxisName;
		    axis = AxDescendant;
		} else if ( str == "parent" ) {
		    token = TkAxisName;
		    axis = AxParent;
		} else if ( str == "ancestor" ) {
		    token = TkAxisName;
		    axis = AxAncestor;
		} else if ( str == "following-sibling" ) {
		    token = TkAxisName;
		    axis = AxFollowingSibling;
		} else if ( str == "preceding-sibling" ) {
		    token = TkAxisName;
		    axis = AxPrecedingSibling;
		} else if ( str == "following" ) {
		    token = TkAxisName;
		    axis = AxFollowing;
		} else if ( str == "preceding" ) {
		    token = TkAxisName;
		    axis = AxPreceding;
		} else if ( str == "attribute" ) {
		    token = TkAxisName;
		    axis = AxAttribute;
		} else if ( str == "namespace" ) {
		    token = TkAxisName;
		    axis = AxNamespace;
		} else if ( str == "self" ) {
		    token = TkAxisName;
		    axis = AxSelf;
		} else if ( str == "descendant-or-self" ) {
		    token = TkAxisName;
		    axis = AxDescendantOrSelf;
		} else if ( str == "ancestor-or-self" ) {
		    token = TkAxisName;
		    axis = AxAncestorOrSelf;
		} else {
		    token = TkError;
		}
	    } else { //if ( lookAhead(1) == '*' ) {
		parsePos += 2;
		token = TkNameTest_NCNameStar;
	    }
	} else {
	    token = TkNameTest_QName;
	}
	return token;
    }

    // we found nothing useful, so it must be an error
    token = TkError;

finished:
    parsePos++;
    return token;
}

/*!
  Returns string representation of token for token where it makes sense
  (e.g. TkFunctionName), otherwise a null-string is returned.
*/
QString QXPathLexicalAnalyzer::getString()
{
    switch ( token ) {
	case TkNameTest_NCNameStar:
	case TkNameTest_QName:
	case TkFunctionName:
	case TkLiteral:
	case TkVariableReference:
	    return str;
	    break;
	default:
	    return QString::null;
	    break;
    }
}

/*!
  Returns the number for TkNumber tokens, otherwise 0 is returned.
*/
double QXPathLexicalAnalyzer::getNumber() const
{
    if ( token == TkNumber )
	return str.toDouble();
    else
	return 0;
}

/*!
  Returns the operator for TkOperator tokens, otherwise OpError is
  returned.
*/
QXPathNS::Operator QXPathLexicalAnalyzer::getOperator() const
{
    if ( token == TkOperator )
	return op;
    else
	return OpError;
}

/*!
  Returns the axis for TkAxisName tokens, otherwise AxError is returned.
*/
QXPathNS::Axis QXPathLexicalAnalyzer::getAxis() const
{
    if ( token == TkAxisName )
	return axis;
    else
	return AxError;
}

/*!
  Returns TRUE if the end of the expression has been reached, otherwise FALSE.
*/
bool QXPathLexicalAnalyzer::atEnd() const
{
    return parsePos >= expr.length();
}

/*!
  Moves the position to the next non-whitespace character.
*/
void QXPathLexicalAnalyzer::eatWs()
{
    while ( !atEnd() && expr[parsePos].isSpace() )
	parsePos++;
}

/*!
  Returns the character that is \a offset away from the actual parsing position
  without moving the position.
*/
QChar QXPathLexicalAnalyzer::lookAhead( int offset ) const
{
    if ( parsePos + offset < expr.length() )
	return expr[parsePos+offset];
    else
	return QChar( 0xFFFF );
}


/***************************************************************
 *
 * QXPathParser
 *
 ***************************************************************/
class QXPathParser : public QXPathNS
{
public:
    QXPathParser();
    ~QXPathParser();

    QXPathAtom* parse( const QString& expr );

private:
    QXPathLexicalAnalyzer *lex;
};


/*!
  Constructor.
*/
QXPathParser::QXPathParser()
{
    lex = 0;
}

/*!
  Destructor.
*/
QXPathParser::~QXPathParser()
{
    delete lex;
}

/*!
  Parses the expression and returns the root of the tree that belongs to the
  expression if no parsing error occurs. Otherwise this function returns 0.
*/
QXPathAtom* QXPathParser::parse( const QString& expr )
{
    Token token;
    QXPathAtom *act_root = 0;

    delete lex;
    lex = new QXPathLexicalAnalyzer( expr );
    while ( !lex->atEnd() ) {
	token = lex->nextToken();
	switch ( token ) {
	    case TkError:
		goto error;
		break;
	    case TkLeftParen:
		break;
	    case TkRightParen:
		break;
	    case TkLeftBracket:
		break;
	    case TkRightBracket:
		break;
	    case TkSelfAbbr:
		break;
	    case TkParentAbbr:
		break;
	    case TkAttribAbbr:
		break;
	    case TkComma:
		break;
	    case TkDoubleColon:
		break;
	    case TkNameTest_Star:
	    case TkNameTest_NCNameStar:
	    case TkNameTest_QName:
		break;
	    case TkNodeType_Comment:
	    case TkNodeType_Text:
	    case TkNodeType_PI:
	    case TkNodeType_Node:
		break;
	    case TkOperator:
		break;
	    case TkFunctionName:
		break;
	    case TkAxisName:
		break;
	    case TkLiteral:
		break;
	    case TkNumber:
		break;
	    case TkVariableReference:
		break;
	}
    }
    delete lex;
    lex = 0;
    return act_root;
error:
    delete act_root;
    delete lex;
    lex = 0;
    return 0;
}


/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
class QXPathPrivate
{
public:
    QXPathPrivate() : exprTree(0)
    {
    }

    ~QXPathPrivate()
    {
	delete exprTree;
    }

private:
    QXPathAtom *exprTree;
    friend class QXPath;
};


/*!
  Creates an empty and therefore invalid XPath.

  \sa isValid() setPath()
*/
QXPath::QXPath()
{
    d = new QXPathPrivate;
}

/*!
  Creates XPath for \a expr. The string \a expr is parsed and stroed in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa isValid() setPath()
*/
QXPath::QXPath( const QString& expr )
{
    d = new QXPathPrivate;
    setExpression( expr );
}

/*!
  Destructor.
*/
QXPath::~QXPath()
{
    delete d;
}

/*!
  Sets the expression \a expr. The string \a expr is parsed and stored in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa expression() isValid()
*/
void QXPath::setExpression( const QString& expr )
{
    QXPathParser parser;
    d->exprTree = parser.parse( expr );
}

/*!
  Returns a string representation of the expression. This string is probably
  different from that one you constructed the expression with. But it is
  semantically equal to that string.

  If the XPath is invalid, a null string is returned.

  \sa setPath(), isValid()
*/
QString QXPath::expression() const
{
    return QString::null;
}

/*!
  Returns TRUE if the XPath is valid, otherwise FALSE. An XPath is invalid when
  it was constructed by a malformed string.
*/
bool QXPath::isValid() const
{
    return d->exprTree != 0;
}


#if 0
// old stuff that I probably don't need
/***************************************************************
 *
 * QXPathStep
 *
 ***************************************************************/
/*!
  Contructs a default location step.
*/
QXPathStep::QXPathStep()
{
    stepAxis = Child;
}

/*!
  Creates an location step with...
*/
QXPathStep::QXPathStep( QXPathStep::Axis axis )
{
    stepAxis = axis;
}

/*!
  Copy constructor.
*/
QXPathStep::QXPathStep( const QXPathStep& step )
{
    stepAxis = step.stepAxis;
}

/*!
  Destructor.
*/
QXPathStep::~QXPathStep()
{
}

/*!
*/
void QXPathStep::setAxis( QXPathStep::Axis axis )
{
    stepAxis = axis;
}

/*!
*/
QXPathStep::Axis QXPathStep::axis() const
{
    return stepAxis;
}
#endif
