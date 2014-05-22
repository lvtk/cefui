
#include <string.h>
#include <cefui/url.h>

using namespace std;

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE url test
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(url_test)
{
    const URL test ("http://lvtoolkit.org/tests/resource.html?key=value&key2=value2#anchor");

    clog << "URL: " << test << std::endl;
    clog << "DOMAIN:    " << test.domain_name() << std::endl;
    clog << "EXTENSION: " << test.file_extension() << std::endl;
    clog << "SCHEME:    " << test.scheme_name() << std::endl;
    clog << "RESOURCE PATH: " << test.resource_path() << std::endl;
    clog << "RESOURCE LEAF: " << test.resource_leaf() << std::endl;
    clog << "QUERY STRING:  " << test.query_string() << std::endl;

    BOOST_REQUIRE (test.anchor() == "anchor");
    BOOST_REQUIRE (test.domain_name() == "lvtoolkit.org");
    BOOST_REQUIRE (test.file_extension() == ".html");
    BOOST_REQUIRE (test.scheme_name() == "http");

    const SchemeType scheme ("HTTP");
    BOOST_CHECK (strcmp (scheme.to_string(), test.scheme_name().c_str()) == 0);
}
