/* Sample program for configure to test STL support on target
platforms.  We are mainly concerned with being able to instantiate
templates for common STL container classes.
*/

#include <iterator>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

int main()
{
    int rval = 0;

    std::vector<int> v1;
    v1.push_back( 0 );
    v1.push_back( 1 );
    v1.push_back( 2 );
    v1.push_back( 3 );
    v1.push_back( 4 );
    if ( v1.size() != 5 ) {
	std::cout << "stltest: vector failed size check, expected 5, got " << v1.size();
	rval = -1;
    }
    if ( v1.capacity() < 5 ) {
	std::cout << "stltest: vector failed capacity check, expected >= 5, got " << v1.capacity();
	rval = -1;
    }
    std::vector<int>::iterator v1it = std::find( v1.begin(), v1.end(), 99 );
    if ( v1it != v1.end() ) {
	std::cout << "stltest: find failed, expected end(), got " << *v1it;
	rval = -1;
    }
    v1it = std::find( v1.begin(), v1.end(), 3 );
    if ( v1it == v1.end() ) {
	std::cout << "stltest: find failed, expected to find element, got end()";
	rval = -1;
    }
    std::vector<int> v2;
    std::copy( v1.begin(), v1it, std::back_inserter( v2 ) );
    if ( v2.size() != 3 ) {
	std::cout << "stltest: copy failed, expected 3 elements, got " << v2.size();
	rval = -1;
    }

    std::map<int, double> m1;
    m1.insert( std::make_pair( 1, 2 ) );
    m1.insert( std::make_pair( 3, 2 ) );
    m1.insert( std::make_pair( 5, 2 ) );
    m1.insert( std::make_pair( 7, 2 ) );
    if ( m1.size() != 4 ) {
	std::cout << "stltest: map failed size check, expected 4, got " << m1.size();
	rval = -1;
    }
    std::map<int,double>::iterator m1it = m1.begin();
    for ( ; m1it != m1.end(); ++m1it ) {
	if ( (*m1it).second != 2 ) {
	    std::cout << "stltest: iterator failed, expected 2, got " << (*m1it).second;
	    rval = -1;
	}
    }
    std::map< int, double > m2( m1 );
    if ( m2.size() != m1.size() ) {
	std::cout << "stltest: map copy failed, expected " << m1.size() << " elements, got " << m2.size();
	rval = -1;
    }

    if ( rval )
	std::cout << std::endl << "returning:" << rval << std::endl;
    return rval;
}
