#include "qxpath.h"

#include "qlist.h"


/***************************************************************
 *
 * QXPathNS: "name space" for the XPath classes
 *
 ***************************************************************/
class QXPathNS {
public:
    enum Token {
	TkError,	// an error occured during parsing
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
	TkAxisName_Child,
	TkAxisName_Descendant,
	TkAxisName_Parent,
	TkAxisName_Ancestor,
	TkAxisName_FollowingSibling,
	TkAxisName_PrecedingSibling,
	TkAxisName_Following,
	TkAxisName_Preceding,
	TkAxisName_Attribute,
	TkAxisName_Namespace,
	TkAxisName_Self,
	TkAxisName_DescendantOrSelf,
	TkAxisName_AncestorOrSelf,
	TkLiteral,
	TkNumber,
	TkVariableReference
    };

    enum Operator {
	OpNone,
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
	FunctionCall
    };

    QXPathAtom( Type t, QXPathAtom *p ) :
	type(t), parent(p)
    {
	children.setAutoDelete( TRUE );
    }

    ~QXPathAtom()
    {
	children.clear();
    }

    Type type;
    QXPathAtom *parent;
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
    QXPathLexicalAnalyzer( const QString &e ) :
	expr(e), parsePos(0), token(TkError)
    {
    }

    ~QXPathLexicalAnalyzer()
    {
    }

    // Tokens are taken from the XPath 1.0 recommendation, section 3.7
    // Returns the next token.
    Token nextToken()
    {
	eatWs();
	if ( atEnd() ) {
	    token = TkError;
	    return token;
	}

	bool forceVariableReference = FALSE;
	QChar parseChar = expr[parsePos];
qDebug( QString(parseChar) );

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
		    str = "*";
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
			token = TkAxisName_Child;
		    } else if ( str == "descendant" ) {
			token = TkAxisName_Descendant;
		    } else if ( str == "parent" ) {
			token = TkAxisName_Parent;
		    } else if ( str == "ancestor" ) {
			token = TkAxisName_Ancestor;
		    } else if ( str == "following-sibling" ) {
			token = TkAxisName_FollowingSibling;
		    } else if ( str == "preceding-sibling" ) {
			token = TkAxisName_PrecedingSibling;
		    } else if ( str == "following" ) {
			token = TkAxisName_Following;
		    } else if ( str == "preceding" ) {
			token = TkAxisName_Preceding;
		    } else if ( str == "attribute" ) {
			token = TkAxisName_Attribute;
		    } else if ( str == "namespace" ) {
			token = TkAxisName_Namespace;
		    } else if ( str == "self" ) {
			token = TkAxisName_Self;
		    } else if ( str == "descendant-or-self" ) {
			token = TkAxisName_DescendantOrSelf;
		    } else if ( str == "ancestor-or-self" ) {
			token = TkAxisName_AncestorOrSelf;
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

    // Returns string representation of token for token where it makes sense
    // (e.g. TkFunctionName), otherwise a null-string is returned.
    QString getString()
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

    // Returns the number for TkNumber tokens, otherwise 0 is returned.
    double getNumber() const
    {
	if ( token == TkNumber )
	    return str.toDouble();
	else
	    return 0;
    }

    // Returns the operator for TkOperator tokens, otherwise OpNone is returned.
    Operator getOperator() const
    {
	if ( token == TkOperator )
	    return op;
	else
	    return OpNone;
    }

    bool atEnd() const
    {
	return parsePos >= expr.length();
    }

private:
    void eatWs()
    {
	while ( !atEnd() && expr[parsePos].isSpace() )
	    parsePos++;
    }

    QChar lookAhead( int offset ) const
    {
	if ( parsePos + offset < expr.length() )
	    return expr[parsePos+offset];
	else
	    return QChar( 0xFFFF );
    }

    QString expr;
    uint parsePos;
    Token token;

    QString str;
    Operator op;
};


/***************************************************************
 *
 * QXPathParser
 *
 ***************************************************************/
class QXPathParser : public QXPathNS
{
public:
    QXPathParser( const QString& expr )
    {
	lex = new QXPathLexicalAnalyzer( expr );
    }

    ~QXPathParser()
    {
	delete lex;
    }

    enum State {
	SExpr,
	SAxisName,
	SNodeTest,
	STestPredicate
    };

    QXPathAtom* parse()
    {
	Token token;
	State s = SExpr;
	QXPathAtom *act_root = 0;

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
		case TkAxisName_Child:
		case TkAxisName_Descendant:
		case TkAxisName_Parent:
		case TkAxisName_Ancestor:
		case TkAxisName_FollowingSibling:
		case TkAxisName_PrecedingSibling:
		case TkAxisName_Following:
		case TkAxisName_Preceding:
		case TkAxisName_Attribute:
		case TkAxisName_Namespace:
		case TkAxisName_Self:
		case TkAxisName_DescendantOrSelf:
		case TkAxisName_AncestorOrSelf:
		    break;
		case TkLiteral:
		    break;
		case TkNumber:
		    break;
		case TkVariableReference:
		    break;
	    }
	}
	return act_root;
    error:
	delete act_root;
	return 0;
    }

private:
    QXPathLexicalAnalyzer *lex;
};


/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
class QXPathPrivate
{
public:
    QXPathPrivate() : expr(0)
    {
    }

    ~QXPathPrivate()
    {
	delete expr;
    }

private:
    QXPathAtom *expr;
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
    QXPathParser parser( expr );
    d->expr = parser.parse();
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
    return d->expr != 0;
}


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
