/*
  trool.h
*/

#ifndef TROOL_H
#define TROOL_H

/*
  A Trool is a bool with three truth values (true, false, and
  default).
*/
enum Trool { Ttrue, Tfalse, Tdef };

inline Trool toTrool( bool b ) {
    return b ? Ttrue : Tfalse;
}

inline bool fromTrool( Trool tr, bool def )
{
    return tr == Tdef ? def : tr == Ttrue;
}

#endif
