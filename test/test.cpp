#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>

#include <sid_canutils/sid_canutils.hpp>

int main( int argc, char** argv )
{
    SID_Interface iface( "can0" );

    std::string first, second;

    while( 1 )
    {
        first = second;
        std::getline( std::cin, second );
        iface.set_upper_str( first );
        iface.set_lower_str( second );
    }

    return 0;
}
