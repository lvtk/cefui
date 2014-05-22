#ifndef CEFUI_CLIENT_APP_H
#define CEFUI_CLIENT_APP_H

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

private:
    IMPLEMENT_REFCOUNTING (ClientApp)
    IMPLEMENT_LOCKING (ClientApp)
};

#endif /* CEFUI_CLIENT_APP_H */
