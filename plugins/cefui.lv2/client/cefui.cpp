
#include <memory>
#include <sstream>
#include <gtk/gtk.h>

#include "lvtk/ui.hpp"
#include "lvtk/ext/idle_interface.hpp"

#include "cefui/url.h"

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"

#include "client/client_app.h"
#include "client/client_controller.h"

#include <X11/Xlib.h>

// directly include the lvtk wrapper
#include "../../../lvtk/src/ui.cpp"

using namespace lvtk;

#define CEFUI_URI     "http://lvtoolkit.org/plugins/cefui"

static bool g_have_cef_init = false;
static int g_num_instances = 0;
static CefRefPtr<ClientApp> g_app;

class CefAmpUI;

typedef UI<CefAmpUI, IdleInterface<false>,
                     URID<true>, PortMap<true>,
                     Parent<true>, UIResize<false> > GuiType;

class CefAmpUI : public GuiType,
                 public ClientController::Listener
{
public:

    CefAmpUI (const char* plugin_uri_)
        : GuiType(),
          m_parent (0), m_window(0),
          m_plugin_uri (plugin_uri_),
          m_browser (-1)
    {       
        chromium_init();

        m_parent = reinterpret_cast<cef_window_handle_t> (get_parent());
        XDisplay* display = cef_get_xdisplay();

        if (m_parent == 0)
        {
            std::clog << "[cefui] using root xwindow as parent.\n";
            m_parent = XRootWindow (display, XDefaultScreen (display));
        }

        m_window = XCreateWindow (display, m_parent,
                                  0, 0, 400, 240, 0,
                                  CopyFromParent,
                                  CopyFromParent,
                                  CopyFromParent,
                                  CopyFromParent,
                                  CopyFromParent);
        XMapWindow (display, m_window);
        sync_browser();
        ui_resize (400, 240);
        ++g_num_instances;
    }

    ~CefAmpUI()
    {
        if (m_client)
            m_client->remove_listener (this);

        XDisplay* display = cef_get_xdisplay();
        XUnmapWindow (display, m_window);
        XDestroyWindow (display, m_window);

        if (have_browser_sync())
            g_app->unregister_browser (m_browser);

        m_browser = -1;
        m_client.reset();

        if (--g_num_instances == 0)
        {
        }
    }

    void client_received_control (const std::string& port, const double value)
    {
        write_control (port_index (port.c_str()), static_cast<float> (value));
    }

    void port_event (uint32_t port, uint32_t size, uint32_t format, void const* buffer)
    {
        if (! have_browser_sync())
            return;

        CefRefPtr<CefProcessMessage> msg (CefProcessMessage::Create (LV2_UI__portNotification));
        CefRefPtr<CefListValue> args (msg->GetArgumentList());
        args->SetInt (0, (int) port);
        args->SetInt (1, (int) format);
        args->SetBinary (2, CefBinaryValue::Create (buffer, (size_t) size));
    }

    /** Execute a snippet of JavaScript
        @param code The javascript code to execute
        @param start_line The line to start executing at
     */
    void execute_javascript (const std::string& code, int start_line = 0)
    {
        if (! have_browser_sync())
            return;
    }

    int idle()
    {
        CefDoMessageLoopWork();
        return 0;
    }

    bool have_browser_sync() const
    {
        return (m_browser >= 0);
    }

    LV2UI_Widget* widget()  {

        return (LV2UI_Widget*)m_window;
    }

private:
    cef_window_handle_t m_parent;
    cef_window_handle_t m_window;
    std::string m_plugin_uri;
    int m_browser;
    std::unique_ptr<ClientController> m_client;


    void chromium_init()
    {
        if (! g_have_cef_init)
        {
            const std::string renderer_path (std::string (bundle_path()).append ("cefui-renderer"));
            const std::string resources_dir (bundle_path());
            const std::string locales_path (resources_dir + std::string ("/locales"));

            CefMainArgs args;
            CefSettings settings;
            settings.single_process = false;
            settings.command_line_args_disabled = true;
            settings.multi_threaded_message_loop = false;
            settings.no_sandbox = false; //TODO: Figure out sandboxing
            CefString (&settings.browser_subprocess_path) = renderer_path;
            CefString (&settings.resources_dir_path) = resources_dir;
            CefString (&settings.locales_dir_path) = locales_path;

           #ifndef NDEBUG
            settings.log_severity = LOGSEVERITY_DISABLE;
           #endif

            if (! g_app)
                g_app = new ClientApp();

            g_have_cef_init = CefInitialize (args, settings, g_app.get(), 0);
        }
    }

    ClientController* get_client_controller()
    {
        if (nullptr == m_client.get())
        {
            m_client.reset (new ClientController());
            m_client->add_listener (this);
        }

        return m_client.get();
    }

    void sync_browser()
    {
        if (ClientController* ctl = get_client_controller())
        {
            m_browser = ctl->create_browser ((void*)m_window, "cefui://plugin/");
        }
        else
        {
            m_browser = -1;
        }

        if (m_browser >= 0)
            g_app->register_browser (m_browser, m_plugin_uri);
        else
            std::clog << "[cefui] error creating/registering a new browser.\n";
    }
};

int cefui_descriptor_id = CefAmpUI::register_class (CEFUI_URI);
