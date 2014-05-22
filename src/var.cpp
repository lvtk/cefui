
#include "cefui/var.h"

var::var() : VariantType ((int32_t) 0) { }
var::var (const int32_t& val) : VariantType (val) { }
var::var (const double& val) : VariantType (val) { }
var::var (const bool& val) : VariantType (val) { }
var::var (const std::string& val) : VariantType (val) { }

bool var::is_bool()    const { return boost::get<bool> (this) != nullptr; }
bool var::is_double()  const { return boost::get<double> (this) != nullptr; }
bool var::is_int()     const { return boost::get<int32_t> (this) != nullptr; }
bool var::is_numeric() const { return (is_bool() || is_string()) ? false : true; }
bool var::is_string()  const { return boost::get<std::string> (this) != nullptr; }
