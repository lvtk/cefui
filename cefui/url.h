
#ifndef CEFUI_URL_H
#define CEFUI_URL_H

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>

class SchemeType
{
public:
    enum ID
    {
        HTTP    = 0,
        HTTPS   = 1,
        File    = 2,
        FTP     = 3
    };

    SchemeType (const int32_t& type) : m_id (type) { }

    SchemeType (const std::string& type) : m_id (-1)
    {
        std::string type_lower;
        type_lower.reserve (type.size());
        std::transform (type.begin(), type.end(), type_lower.begin(), tolower);

        for (int32_t i = HTTP; i <= FTP; ++i)
            if (std::strcmp (to_string (i), type_lower.c_str()) == 0)
                { m_id = i; break; }
    }

    ~SchemeType() { }

    static const char* to_string (int32_t type)
    {
        static const char* g_scheme_slugs[4] = {
            "http", "https", "file", "ftp"
        };

        return type <= SchemeType::FTP ? g_scheme_slugs [type] : nullptr;
    }

    const char* to_string() const {
        return SchemeType::to_string (m_id);
    }

private:
    int32_t m_id;
};


class URL : public std::string
{
public:

    /** Create an empty, invalid, URL */
    URL();

    /** Create a URL from a string source */
    URL (const std::string& u);

    /** dtor */
    ~URL();

    /** Returns the URL's query string */
    std::string query_string() const;

    /** Returns true if the URL has an anchor component */
    bool has_anchor() const;

    /** Returns the anchor component */
    std::string anchor() const;

    /** Returns the domain name */
    std::string domain_name() const;

    /** Returns the file extension or empty if none */
    std::string file_extension() const;

    /** Returns true if this is a URL with a 'file' scheme */
    bool is_file() const;

    /** Returns true if this is the root, '/', path */
    bool is_root() const;

    /** Returns the filename only */
    std::string resource_leaf() const;

    /** Returns the full resource path */
    std::string resource_path() const;

    /** Returns the scheme name, e.g. http */
    std::string scheme_name() const;
};

#endif // CEFUI_URL_H
