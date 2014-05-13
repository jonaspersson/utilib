#include <boost/test/unit_test.hpp>

using namespace boost::unit_test;
void
test_measure_scope()
{
         BOOST_CHECK( false );
}


test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
    test_suite* test = BOOST_TEST_SUITE( "Master test suite" );

    test->add( BOOST_TEST_CASE( &test_measure_scope ) );

    return test;
}
