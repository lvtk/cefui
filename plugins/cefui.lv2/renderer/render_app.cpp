
#include <iostream>
#include <string>

#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

#include "include/cef_v8.h"
#include "include/capi/cef_v8_capi.h"
#include "renderer/render_app.h"


class MyV8Handler : public CefV8Handler {
public:
    MyV8Handler (CefRefPtr<CefBrowser> b) : browser(b){}
    virtual ~MyV8Handler() {
    }

    virtual bool Execute (const CefString& name,
                          CefRefPtr<CefV8Value> object,
                          const CefV8ValueList& arguments,
                          CefRefPtr<CefV8Value>& retval,
                          CefString& exception) override;

    // Provide the reference counting implementation for this class.
    IMPLEMENT_REFCOUNTING (MyV8Handler)

    private:
        CefRefPtr<CefBrowser> browser;
};

bool MyV8Handler::Execute (const CefString& name,
                           CefRefPtr<CefV8Value> object,
                           const CefV8ValueList& arguments,
                           CefRefPtr<CefV8Value>& retval,
                           CefString& exception)
{
    if (name == "_writeControl")
    {
        // Return my string value.
        CefRefPtr<CefV8Value> port = arguments[0];
        CefRefPtr<CefV8Value> val  = arguments[1];

        if (port->IsString() && val->IsDouble())
        {
            const std::string sval = port->GetStringValue();
            const double value = val->GetDoubleValue();
            retval = CefV8Value::CreateBool (true);

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create ("Control.Value");
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetDouble (0, value);
            args->SetString (1, sval);
            browser->SendProcessMessage (PID_BROWSER, msg);

        } else {
            retval = CefV8Value::CreateBool (false);
        }

        return true;
    }

    // Function does not exist.
    return false;
}


RenderApp::RenderApp() { }
RenderApp::~RenderApp() { }

// render process handler
void RenderApp::OnContextCreated (CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       CefRefPtr<CefV8Context> context)
{
    m_router->OnContextCreated (browser, frame, context);
}

void RenderApp::OnContextReleased (CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context)
{
    m_router->OnContextReleased (browser, frame, context);
}

void RenderApp::OnWebKitInitialized()
{
    CefMessageRouterConfig config;
    m_router = CefMessageRouterRendererSide::Create (config);
}

void RenderApp::OnBrowserCreated (CefRefPtr<CefBrowser> browser)
{
#if 1
    std::string extensionCode =
       "var lvtk;"
       "if (! lvtk)"
       "  lvtk = {"
       "    ui: {}"
       "};"
       "(function() {"
       "  lvtk.ui.writeControl = function (a1, a2) {"
       "    native function _writeControl();"
       "    return _writeControl (a1.toString(), parseFloat (a2));"
       "  };"
       "  lvtk.ui.write = function () { };"
       "})();";

     // Create an instance of my CefV8Handler object.
     CefRefPtr<CefV8Handler> handler = new MyV8Handler (browser);

     // Register the extension.
     CefRegisterExtension ("v8/lvtk", extensionCode, handler);
#endif
}

void RenderApp::OnBrowserDestroyed (CefRefPtr<CefBrowser> browser) { }

bool RenderApp::OnProcessMessageReceived (CefRefPtr<CefBrowser> browser, CefProcessId source,
                                       CefRefPtr<CefProcessMessage> message)
{
    if (m_router->OnProcessMessageReceived (browser, source, message))
        return true;

    CefRefPtr<CefListValue> args (message->GetArgumentList());

    if (message->GetName() == LV2_UI__portNotification)
    {
        const int port = args->GetInt (0);
        const int format = args->GetInt (1);

        if (format == 0)
        {
            CefRefPtr<CefBinaryValue> bin (args->GetBinary (2));
            float control_value;
            bin->GetData (&control_value, bin->GetSize(), 0);
#if 1
            browser->GetMainFrame()->GetV8Context()->Enter();
            CefRefPtr<CefV8Value> ctx (browser->GetMainFrame()->GetV8Context()->GetGlobal());

            CefRefPtr<CefV8Value> lvtk = ctx->GetValue ("lvtk");

            if (lvtk && lvtk->IsObject())
            {
                CefRefPtr<CefV8Value> func = lvtk->GetValue ("testFunction");
                CefV8ValueList args;

                if (func->IsFunction() && func->IsValid()) {
                    if (! func->ExecuteFunction (lvtk, args))
                        std::clog << "couldn't execute func\n";
                }
                else {
                    std::clog << "not a function\n";
                }
            } else {
                std::clog << "lvtk object not found\n";
            }
            browser->GetMainFrame()->GetV8Context()->Exit();
#endif
        }

        return true;
    }

    return false;
}
