#ifndef CEFUI_CLIENT_APP_H
#define CEFUI_CLIENT_APP_H

#include <memory>
#include <string>
#include <unordered_map>

#include "include/cef_app.h"

/** Implements the CefApp interface for browser-side handling */
class ClientApp :  public CefApp,
                   public CefBrowserProcessHandler
{
public:
    ClientApp();
    ~ClientApp();

    /** Register a plugin for a given browser id
        @param browser_id The browser ID. @see CefBrowser::GetIdentifier()
        @param plugin_uri The URI of the LV2 Plugin the browser controls
     */
    void register_browser (int browser_id, const std::string& plugin_uri);

    /** Unregister a browser
        @param browser_id The browser to remove
     */
    void unregister_browser (int browser_id);

private:
    typedef std::unordered_map<int, const std::string> PathMap;
    PathMap m_paths;

    // Cef overrides
    virtual void OnContextInitialized() override;
    virtual void OnRegisterCustomSchemes (CefRefPtr<CefSchemeRegistrar> registrar) override;
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    friend class SchemeHandlerFactory;

    class Impl; friend class Impl;
    std::unique_ptr<Impl> impl;

    IMPLEMENT_REFCOUNTING (ClientApp)
    IMPLEMENT_LOCKING (ClientApp)
};

#endif /* CEFUI_CLIENT_APP_H */
