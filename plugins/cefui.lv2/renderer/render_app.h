
#ifndef CEFUI_RENDER_APP_H
#define CEFUI_RENDER_APP_H

#include "include/wrapper/cef_message_router.h"
#include "include/cef_app.h"

class RenderApp :  public CefApp,
                   public CefRenderProcessHandler
{
public:

    RenderApp();
    ~RenderApp();

    // render process handler
    void OnContextCreated (CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override;

    void OnContextReleased (CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefV8Context> context);

    void OnWebKitInitialized() override;

    void OnBrowserCreated (CefRefPtr<CefBrowser> browser) override;

    virtual void OnBrowserDestroyed (CefRefPtr<CefBrowser> browser);

    virtual bool OnProcessMessageReceived (CefRefPtr<CefBrowser> browser, CefProcessId source,
                                           CefRefPtr<CefProcessMessage> message);

    // CefApp Handlers methods:
    CefRefPtr<CefRenderProcessHandler>  GetRenderProcessHandler()  override { return this; }

private:
    CefRefPtr<CefMessageRouterRendererSide> m_router;
    IMPLEMENT_REFCOUNTING (RenderApp)
};


#endif // RENDER_APP_H
