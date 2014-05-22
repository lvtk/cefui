
#include <iostream>
#include <sstream>

#include <lilv/lilv.h>

#include "cefui/var.h"
#include "cefui/url.h"

#include "include/wrapper/cef_stream_resource_handler.h"

#include "common/util.h"
#include "client/client_app.h"



using namespace std;


static void cefui_get_bundle_path (string& path, const string& plugin_uri)
{
    LilvWorld* world = lilv_world_new();
    LilvNode* bundle = lilv_new_uri (world, plugin_uri.c_str());
    lilv_world_load_all (world);

    const LilvPlugins* plugins = lilv_world_get_all_plugins (world);
    const LilvPlugin*  plugin = lilv_plugins_get_by_uri (plugins, bundle);
    path = lilv_uri_to_path (lilv_node_as_uri (lilv_plugin_get_bundle_uri (plugin)));
    path.append ("content/");
    lilv_node_free (bundle);
    lilv_world_free (world);
}

// Implementation of the factory for creating client request handlers.
class SchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
    virtual CefRefPtr<CefResourceHandler> Create (CefRefPtr<CefBrowser> browser,
                                                  CefRefPtr<CefFrame> frame,
                                                  const CefString& scheme_name,
                                                  CefRefPtr<CefRequest> request) override
    {
        const URL url (request->GetURL());
        //string path ("/home/mfisher/workspace/gnu/cefui/build/cefamp.lv2/content/");
        string path;
        cefui_get_bundle_path (path, "http://lvtoolkit.org/plugins/cefamp");
        path.append (url.resource_path());
        if (url.file_extension().empty())
            path.append ("index.html");

        CefRefPtr<CefStreamReader> stream (CefStreamReader::CreateForFile (path));

        if (stream)
            return new CefStreamResourceHandler ("text/html", stream);

        return nullptr;
    }

private:
    IMPLEMENT_REFCOUNTING (SchemeHandlerFactory)
};


ClientApp::ClientApp() { }
ClientApp::~ClientApp() { }

// browser process handler
void ClientApp::OnContextInitialized()
{
    CefRegisterSchemeHandlerFactory ("cefui", "plugin", new SchemeHandlerFactory());
}

void ClientApp::OnRegisterCustomSchemes (CefRefPtr<CefSchemeRegistrar> registrar)
{
    registrar->AddCustomScheme ("cefui", true, false, false);
}
