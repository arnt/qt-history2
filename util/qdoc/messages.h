/*
  messages.h
*/

#ifndef MESSAGES_H
#define MESSAGES_H

class Location;

void setMaxSimilarMessages( int n );
void setMaxMessages( int n );
void setWarningLevel( int level );
void setParanoiaEnabled( bool on );

void warning( int level, const Location& loc, const char *message, ... );
void warning( int level, const char *message, ... );
void syswarning( const char *message, ... );
void warnAboutOmitted();

#endif
