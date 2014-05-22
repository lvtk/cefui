#ifndef CEFUI_VAR_H
#define CEFUI_VAR_H

#include <stdint.h>
#include <string>
#include <boost/variant.hpp>


typedef boost::variant<int32_t, double, bool, std::string> VariantType;

class var : public VariantType
{
public:
    var();
    var (const int32_t& val);
    var (const double& val);
    var (const bool& val);
    var (const std::string& val);

    bool is_bool() const;
    bool is_double() const;
    bool is_int() const;
    bool is_numeric() const;
    bool is_string() const;
};


#endif // CEFUI_VAR_H
