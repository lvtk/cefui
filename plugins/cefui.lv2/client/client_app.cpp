
#include <iostream>
#include <sstream>

#include <lilv/lilv.h>

#include "cefui/var.h"
#include "cefui/url.h"

#include "include/wrapper/cef_stream_resource_handler.h"

#include "common/util.h"
#include "client/client_app.h"

using namespace std;

static LilvWorld* g_lilv_world = nullptr;

static void cefui_plugin_get_path (string& path, const string& plugin_uri)
{
    LilvWorld* const world = g_lilv_world;

    LilvNode* bundle = lilv_new_uri (world, plugin_uri.c_str());
    const LilvPlugins* plugins = lilv_world_get_all_plugins (world);
    const LilvPlugin*  plugin  = lilv_plugins_get_by_uri (plugins, bundle);
    path = lilv_uri_to_path (lilv_node_as_uri (lilv_plugin_get_bundle_uri (plugin)));
    path.append ("content/");

    lilv_node_free (bundle);
}

// Implementation of the factory for creating client request handlers.
class SchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:

    explicit SchemeHandlerFactory (ClientApp* app) : p_app (app) { }

    virtual CefRefPtr<CefResourceHandler> Create (CefRefPtr<CefBrowser> browser,
                                                  CefRefPtr<CefFrame> frame,
                                                  const CefString& scheme_name,
                                                  CefRefPtr<CefRequest> request) override
    {
        const URL url (request->GetURL());
        string path (p_app->m_browser_map [browser->GetIdentifier()]);
        path.append (url.resource_path());
        if (url.file_extension().empty())
            path.append ("index.html");

        if (CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForFile (path))
            return new CefStreamResourceHandler ("text/html", stream);

        return nullptr;
    }

private:
    ClientApp* p_app;
    IMPLEMENT_REFCOUNTING (SchemeHandlerFactory)
};


ClientApp::ClientApp()
{
    g_lilv_world = lilv_world_new();
    lilv_world_load_all (g_lilv_world);
}

ClientApp::~ClientApp()
{
    lilv_world_free (g_lilv_world);
    g_lilv_world = nullptr;
}

// browser process handler
void ClientApp::OnContextInitialized()
{
    CefRegisterSchemeHandlerFactory ("cefui", "plugin", new SchemeHandlerFactory (this));
}

void ClientApp::OnRegisterCustomSchemes (CefRefPtr<CefSchemeRegistrar> registrar)
{
    registrar->AddCustomScheme ("cefui", true, false, false);
}

void ClientApp::RegisterBrowserForPlugin (int browser_id, const std::string& plugin_uri)
{
    if (m_browser_map.end() != m_browser_map.find (browser_id))
        return;

    string plugin_path;
    cefui_plugin_get_path (plugin_path, plugin_uri);
    m_browser_map.insert (make_pair (browser_id, plugin_path));
}
