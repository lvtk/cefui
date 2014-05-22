
#include "cefui/url.h"


URL::URL() : std::string() { }

URL::URL (const std::string& u)
    : std::string (u) { }

URL::~URL() { }

std::string URL::query_string() const {
    const size_type it = find_first_of ("?");
    return it != npos ? substr (it) : std::string();
}

bool URL::has_anchor() const
{
    const size_type it = find_first_of ("?");
    return find_last_of ("#", it) != npos;
}

std::string URL::anchor() const
{
    const size_type pos = find_last_of ("#");
    return pos != npos ? substr (pos + 1) : std::string();
}

std::string URL::domain_name() const
{
    const size_type start = 3 + find_first_of ("://");
    return substr (start, find_first_of ("/", start) - start);
}

std::string URL::file_extension() const
{
    const size_type start = find_last_of (".");
    return (start != npos) ? substr (start, find_first_of ("?", start) - start)
                           : std::string();
}

bool URL::is_file() const
{
    return find_first_of ("file://") != npos;
}

bool URL::is_root() const
{
    const std::string res (resource_path());
    return res.empty() || res == "/";
}

std::string URL::resource_leaf() const
{
    const size_type start = 1 + find_last_of ("/");
    return substr (start, find_first_of ("#", start) - start);
}

std::string URL::resource_path() const
{
    const size_type start = find_first_of ("/", 3U + find_first_of ("://"));
    const size_type end = find_first_of ("?", start);
    return substr (start, end - start);
}

std::string URL::scheme_name() const
{
    const size_type pos = find_first_of (":");
    return pos != URL::npos ? substr (0, pos) : std::string();
}
