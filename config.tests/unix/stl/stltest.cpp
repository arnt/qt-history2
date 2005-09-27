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
    std::vector<int> v1;
    v1.push_back( 0 );
    v1.push_back( 1 );
    v1.push_back( 2 );
    v1.push_back( 3 );
    v1.push_back( 4 );
    int v1size = v1.size();
    v1size = 0;
    int v1capacity = v1.capacity();
    v1capacity = 0;

    std::vector<int>::iterator v1it = std::find( v1.begin(), v1.end(), 99 );
    bool v1notfound = (v1it == v1.end());
    v1notfound = false;

    v1it = std::find( v1.begin(), v1.end(), 3 );
    bool v1found = (v1it != v1.end());
    v1found = false;

    std::vector<int> v2;
    std::copy( v1.begin(), v1it, std::back_inserter( v2 ) );
    int v2size = v2.size();
    v2size = 0;

    std::map<int, double> m1;
    m1.insert( std::make_pair( 1, 2.0 ) );
    m1.insert( std::make_pair( 3, 2.0 ) );
    m1.insert( std::make_pair( 5, 2.0 ) );
    m1.insert( std::make_pair( 7, 2.0 ) );
    int m1size = m1.size();
    m1size = 0;
    std::map<int,double>::iterator m1it = m1.begin();
    for ( ; m1it != m1.end(); ++m1it ) {
        int first = (*m1it).first;
        first = 0;
        double second = (*m1it).second;
        second = 0.0;
    }
    std::map< int, double > m2( m1 );
    int m2size = m2.size();
    m2size = 0;

    return 0;
}

// something mean to see if the compiler and C++ standard lib are good enough
template<class K, class T>
class DummyClass
{
    // everything in std namespace ?
    typedef std::bidirectional_iterator_tag i;
    typedef std::ptrdiff_t d;
    // typename implemented ?
    typedef typename std::map<K,T>::iterator MyIterator;
};
