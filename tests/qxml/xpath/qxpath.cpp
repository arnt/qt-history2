#include "qxpath.h"


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
	    num = str.toDouble();
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
	    return num;
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
    double num;
    Operator op;
};

/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
/*!
  Creates an empty and therefore invalid XPath.

  \sa isValid() setPath()
*/
QXPath::QXPath()
{
    d = 0;
}

/*!
  Creates XPath for \a expr. The string \a expr is parsed and stroed in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa isValid() setPath()
*/
QXPath::QXPath( const QString& expr )
{
    d = 0;
    setExpression( expr );
}

/*!
  Destructor.
*/
QXPath::~QXPath()
{
}


/*!
  Sets the expression \a expr. The string \a expr is parsed and stored in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa expression() isValid()
*/
void QXPath::setExpression( const QString& expr )
{
    parse( expr );
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
    return TRUE;
}

/*!
  Parses the string \a expr and returns TRUE if the parsing was ok, otherwise
  returns FALSE.
*/
bool QXPath::parse( const QString& expr )
{
#if 0
    QString p = expr.stripWhiteSpace();
    int pos = 0;

    if ( p[0] == '/' ) {
	pos ++;
    } else {
    }
#endif
    QXPathLexicalAnalyzer lex( expr );
    QXPathNS::Token token;
qDebug( "Reporting tokens for: %s", expr.latin1() );
    while ( !lex.atEnd() ) {
	token = lex.nextToken();
qDebug( "Token %d: (%s) (%f) (%d)", token,
	lex.getString().latin1(), lex.getNumber(), lex.getOperator() );
    }

    return TRUE;
}


/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
/*!
  Contructs a default location step.
*/
QXPathStep::QXPathStep()
{
    stepAxis = QXPath::Child;
}

/*!
  Creates an location step with...
*/
QXPathStep::QXPathStep( QXPath::Axis axis )
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
void QXPathStep::setAxis( QXPath::Axis axis )
{
    stepAxis = axis;
}

/*!
*/
QXPath::Axis QXPathStep::axis() const
{
    return stepAxis;
}
