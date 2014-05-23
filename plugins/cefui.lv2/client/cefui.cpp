
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


// directly include the lvtk wrapper
#include "../../../lvtk/src/ui.cpp"

using namespace lvtk;

#define CEFUI_URI     "http://lvtoolkit.org/plugins/cefui"

static gint timeout_callback (gpointer data);
static guint g_timeout = 0;
static bool g_have_cef_init = false;
static int g_num_instances = 0;
static CefRefPtr<ClientApp> g_app;

class CefAmpUI;

typedef UI<CefAmpUI, IdleInterface<false>,
                     URID<true>, PortMap<true> > GuiType;

class CefAmpUI : public GuiType,
                 public ClientController::Listener
{
public:

    CefAmpUI (const char* plugin_uri_)
        : GuiType()
    {
        m_browser = -1;
        m_plugin_uri = plugin_uri_;

        p_vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_set_size_request (p_vbox, 420, 240);

        chromium_init();
        sync_browser();

        ++g_num_instances;
    }

    ~CefAmpUI()
    {
        if (m_client)
            m_client->remove_listener (this);

        if (have_browser_sync())
            g_app->unregister_browser (m_browser);

        m_browser = -1;
        m_client.reset();

        // gtk_widget_destroy (p_vbox);
        p_vbox = nullptr;

        if (--g_num_instances == 0)
        {
            if (g_timeout != 0) {
                gtk_timeout_remove (g_timeout);
                g_timeout = 0;
            }
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
        return 0;
    }

    bool have_browser_sync() const
    {
        return (m_browser >= 0);
    }

    LV2UI_Widget* widget()  {
        return (LV2UI_Widget*)p_vbox;
    }

private:
    std::string m_plugin_uri;
    GtkWidget* p_vbox;
    std::unique_ptr<ClientController> m_client;
    int m_browser;

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
            settings.no_sandbox = true; //TODO: Figure out sandboxing
            CefString (&settings.browser_subprocess_path) = renderer_path;
            CefString (&settings.resources_dir_path) = resources_dir;
            CefString (&settings.locales_dir_path) = locales_path;

           #ifndef NDEBUG
            settings.log_severity = LOGSEVERITY_DISABLE;
           #endif

            std::clog << "CREATING CLIENT APP. CEF3 INIT\n";
            g_app = new ClientApp();
            g_have_cef_init = CefInitialize (args, settings, g_app.get(), 0);
        }

        if (! g_have_cef_init)
            return;

        if (g_timeout == 0)
            g_timeout = gtk_timeout_add (30, &timeout_callback, nullptr);

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
            m_browser = ctl->create_browser (p_vbox, "cefui://plugin/");
        }
        else
        {
            m_browser = -1;
        }

        if (m_browser >= 0)
            g_app->register_browser (m_browser, m_plugin_uri);
    }
};


int cefui_desc_id = CefAmpUI::register_class (CEFUI_URI);

#ifdef __linux__
gint timeout_callback (gpointer)
{
    if (! g_have_cef_init)
        return FALSE;

    CefDoMessageLoopWork();
    return TRUE;
}
#endif
