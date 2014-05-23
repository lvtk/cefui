
#include <algorithm>
#include <sstream>
#include <string>

#include "include/cef_app.h"
#include "include/cef_runnable.h"

#include "client/client_controller.h"
#include "common/util.h"

typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
typedef std::list<ClientController::Listener*> ListenerList;

class ClientMenu
{
public:
    // Custom menu command Ids.
    enum ItemID
    {
        ShowDevTools   = MENU_ID_USER_FIRST,
        CloseDevTools
    };
};

class TestQuery : public CefMessageRouterBrowserSide::Handler
{
public:
    TestQuery() { }
    virtual ~TestQuery() { }

    // Called due to cefQuery execution in binding.html.
    bool OnQuery (CefRefPtr<CefBrowser> browser,
                  CefRefPtr<CefFrame> frame,
                  int64 query_id,
                  const CefString& request,
                  bool persistent,
                  CefRefPtr<Callback> callback) override
    {
        // Only handle messages from the test URL.
        const std::string& url = frame->GetURL();
        //if (url.find (kTestUrl) != 0)
          //  return false;

        const std::string& message_name = request;
        //if (message_name.find (kTestMessageName) == 0)
        {
            // Reverse the string and return.
           // std::string result = message_name.substr(sizeof(kTestMessageName));
            //std::reverse(result.begin(), result.end());
            callback->Success ("result");
            return true;
        }

        return false;
    }
};

static int g_num_browsers = 0;







class ClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler
{
public:
    typedef std::set<CefMessageRouterBrowserSide::Handler*> MessageHandlerSet;

    ClientHandler()
        : m_is_closing (false)
    {
    }

    ~ClientHandler() { }


    void OnBeforeContextMenu (CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefContextMenuParams> params,
                              CefRefPtr<CefMenuModel> model)
    {
        if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0)
        {
            // Add a separator if the menu already has items.
            if (model->GetCount() > 0)
                model->AddSeparator();

            // Add DevTools items to all context menus.
            model->AddItem (ClientMenu::ShowDevTools, "&Show DevTools");
            model->AddItem (ClientMenu::CloseDevTools, "Close DevTools");
        }
    }

    bool OnContextMenuCommand (CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefContextMenuParams> params,
                               int command,
                               EventFlags flags)
    {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings;

        switch (command)
        {
            case ClientMenu::ShowDevTools:
                browser->GetHost()->ShowDevTools (windowInfo, this, settings);
                return true;
            case ClientMenu::CloseDevTools:
                browser->GetHost()->CloseDevTools();
                return true;
            default:  // Allow default handling, if any.
                break;
        }

        return false;
    }


    void OnContextMenuDismissed (CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame)
    {

    }


    bool OnProcessMessageReceived (CefRefPtr<CefBrowser> browser,
                                   CefProcessId source,
                                   CefRefPtr<CefProcessMessage> message)
    {
        if (m_router->OnProcessMessageReceived (browser, source, message))
            return true;

        if (PID_RENDERER != source)
            return false;

        if (message->GetName() == "Control.Value")
        {
            double value = 0.0f; //message->GetArgumentList()->GetDouble (0);
            std::string port;

            for (int i = 0; i < message->GetArgumentList()->GetSize(); ++i)
            {
                if (message->GetArgumentList()->GetType(i) == VTYPE_DOUBLE)
                    value = message->GetArgumentList()->GetDouble(i);
                else if (message->GetArgumentList()->GetType(i) == VTYPE_STRING)
                    port = message->GetArgumentList()->GetString(i);

            }

            for (ClientController::Listener* listener : listeners)
                listener->client_received_control (port, value);
        }
        else {
            std::clog << std::string("message received: ") << browser->GetIdentifier()
                      << std::string (" ") << std::string (message->GetName()) << "\n";
        }

        return true;
    }

    void OnAfterCreated (CefRefPtr<CefBrowser> browser)
    {
        REQUIRE_UI_THREAD();
        if (! m_router)
        {
            // Create the browser-side router for query handling.
            CefMessageRouterConfig config;
            m_router = CefMessageRouterBrowserSide::Create (config);

            // Register handlers with the router.
            // TODO: CreateMessageHandlers (m_message_handlers);
            m_message_handlers.clear();
            m_message_handlers.insert (new TestQuery());
            MessageHandlerSet::const_iterator it = m_message_handlers.begin();
            for (; it != m_message_handlers.end(); ++it)
                m_router->AddHandler (*(it), false);
        }

        // Disable mouse cursor change if requested via the command-line flag.
        // if (m_bMouseCursorChangeDisabled)
        // browser->GetHost()->SetMouseCursorChangeDisabled(true);

        AutoLock sl (this);

        if (! m_browser.get())
        {
            // We need to keep the main child window, but not popup windows
            m_browser = browser;
            m_browser_id = browser->GetIdentifier();
        }
        else if (browser->IsPopup())
        {
            // Add to the list of popup browsers.
            popups.push_back (browser);
        }

        g_num_browsers++;
    }

    bool DoClose (CefRefPtr<CefBrowser> browser)
    {
        REQUIRE_UI_THREAD();


        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed destription of this
        // process.
        if (m_browser_id == browser->GetIdentifier())
        {
            // Set a flag to indicate that the window close should be allowed.
            m_is_closing = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

    void OnBeforeClose (CefRefPtr<CefBrowser> browser)
    {
        REQUIRE_UI_THREAD();

        m_router->OnBeforeClose (browser);

        // Remove from the list of existing browsers.
        BrowserList::iterator bit = popups.begin();
        for (; bit != popups.end(); ++bit) {
            if ((*bit)->IsSame(browser))
            {
                popups.erase(bit);
                break;
            }
        }

        if (popups.empty()) {
            // All browser windows have closed. Quit the application message loop.
            // CefQuitMessageLoop();
        }
    }

    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl)
    {
        REQUIRE_UI_THREAD();

        // Don't display an error for downloaded files.
        if (errorCode == ERR_ABORTED)
            return;

        // Display a load error message.
        std::stringstream ss;
        ss << "<html><body bgcolor=\"white\">"
              "<h2>Failed to load URL " << std::string(failedUrl) <<
              " with error " << std::string(errorText) << " (" << errorCode <<
              ").</h2></body></html>";

        frame->LoadString (ss.str(), failedUrl);
    }

    void OnTitleChange (CefRefPtr<CefBrowser>, const CefString&) {
        REQUIRE_UI_THREAD();
    }

    CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }
    CefRefPtr<CefDisplayHandler>     GetDisplayHandler()     override { return this; }
    CefRefPtr<CefLifeSpanHandler>    GetLifeSpanHandler()    override { return this; }
    CefRefPtr<CefLoadHandler>        GetLoadHandler()        override { return this; }
    CefRefPtr<CefRequestHandler>     GetRequestHandler()     override { return this; }

private:
    CefRefPtr<CefBrowser> m_browser;
    int m_browser_id;


    BrowserList popups;
    bool m_is_closing;

    CefRefPtr<CefMessageRouterBrowserSide> m_router;
    MessageHandlerSet m_message_handlers;
    ListenerList listeners;

    IMPLEMENT_LOCKING (ClientHandler)
    IMPLEMENT_REFCOUNTING (ClientHandler)

    friend class ClientController::Impl;
};


class ClientController::Impl {
public:

    Impl (ClientController* c) : client(c)
    {
        handler = new ClientHandler();
    }

    ~Impl()
    {
        handler = 0;
        client = 0;
    }

    void close_all_browsers (const bool force)
    {
        if (handler->m_browser)
            handler->m_browser->GetHost()->CloseBrowser (force);

        if (handler->popups.empty())
            return;

        BrowserList::const_iterator it = handler->popups.begin();
        for (; it != handler->popups.end(); ++it)
            (*it)->GetHost()->CloseBrowser (force);
    }

    void add_listener (ClientController::Listener* listener)
    {
        ListenerList& listeners (handler->listeners);
        if (std::find (listeners.begin(), listeners.end(), listener) == listeners.end())
            listeners.push_back (listener);
    }

    void remove_listener (ClientController::Listener* listener)
    {
        ListenerList& listeners (handler->listeners);
        ListenerList::iterator it = std::find (listeners.begin(),
                                               listeners.end(), listener);
        if (it != listeners.end())
            listeners.erase (it);
    }

    CefRefPtr<ClientHandler> get_client_handler() const {
        return handler;
    }

private:
    typedef ClientHandler::AutoLock AutoLock;
    ClientController* client;
    CefRefPtr<ClientHandler> handler;
};


ClientController::ClientController()
{
    impl.reset (new Impl (this));
}

ClientController::~ClientController()
{
    impl->close_all_browsers (true);
    impl.reset();
}

void
ClientController::add_listener (Listener* l)
{
    impl->add_listener (l);
}

void
ClientController::remove_listener (Listener* l)
{
    impl->remove_listener (l);
}

void ClientController::close_all_browsers (bool force)
{
    impl->close_all_browsers (force);
}

int
ClientController::create_browser (void* widget, const std::string& url)
{
    CefWindowInfo info;
    if (nullptr != widget)
        info.SetAsChild (reinterpret_cast<cef_window_handle_t> (widget));

    CefBrowserSettings settings;
    const std::string destination (url.empty() ? "http://lv2plug.in" : url);

    CefRefPtr<CefBrowser> browser (CefBrowserHost::CreateBrowserSync (
        info, impl->get_client_handler().get(),
        destination, settings, 0));

    return browser ? browser->GetIdentifier() : -1;
}
