#ifndef CEFUI_CLIENT_APP_H
#define CEFUI_CLIENT_APP_H

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
    virtual void OnContextInitialized() override;
    virtual void OnRegisterCustomSchemes (CefRefPtr<CefSchemeRegistrar> registrar) override;
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this;}
    void RegisterBrowserForPlugin (int browser_id, const std::string& plugin_uri);

private:
    IMPLEMENT_REFCOUNTING (ClientApp)
    IMPLEMENT_LOCKING (ClientApp)
    std::unordered_map<int, const std::string> m_browser_map;
    friend class SchemeHandlerFactory;
};

#endif /* CEFUI_CLIENT_APP_H */
