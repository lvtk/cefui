#ifndef CEFUI_RESOURCE_HANDLER_H
#define CEFUI_RESOURCE_HANDLER_H

#include "include/cef_resource_handler.h"

class MyResourceHandler : public CefResourceHandler
{
    string data_;
    string mime_type_;
    string::size_type offset_;

public:
    MyResourceHandler()
    {
        offset_ = 0;
        mime_type_ = data_ = string();
    }

    virtual bool ProcessRequest (CefRefPtr<CefRequest> request,
                                 CefRefPtr<CefCallback> callback) override
    {
        REQUIRE_IO_THREAD();

        bool handled = false;
        AutoLock sl (this);

        const string url (request->GetURL());

       // if (strstr (url.c_str(), "index.html") != nullptr)
        {
            // Build the response html
            data_ = "<html><head><title>Client Scheme Handler</title></head>"
                    "<body bgcolor=\"white\">"
                    "This contents of this page page are served by the "
                    "ClientSchemeHandler class handling the client:// protocol."
                    "<br/>You should see an image:"
                    "<br/><img src=\"client://tests/logo.png\"><pre>";

            // Output a string representation of the request
            //string dump;
            //DumpRequestContents(request, dump);
            //data_.append(dump);

            data_.append ("</pre><br/>Try the test form:"
                          "<form method=\"POST\" action=\"handler.html\">"
                          "<input type=\"text\" name=\"field1\">"
                          "<input type=\"text\" name=\"field2\">"
                          "<input type=\"submit\">"
                          "</form></body></html>");

            handled = true;

            mime_type_ = "text/html";
        }

        if (handled) {
            // Indicate the headers are available.
            callback->Continue();
        }

        return handled;
    }

    virtual void GetResponseHeaders (CefRefPtr<CefResponse> response,
                                     int64& response_length,
                                     CefString& /*redirectUrl*/) override
    {
        ASSERT (! data_.empty());

        response->SetMimeType (mime_type_);
        response->SetStatus (200);
        response_length = data_.length();
    }

    virtual void Cancel() override {
         REQUIRE_IO_THREAD();
    }

    virtual bool ReadResponse (void* data_out, int bytes_to_read,
                               int& bytes_read, CefRefPtr<CefCallback> callback) override
    {
        REQUIRE_IO_THREAD();

        bool has_data = false;
        bytes_read = 0;

        AutoLock sl (this);

        if (offset_ < data_.length())
        {
            // Copy the next block of data into the buffer.
            int transfer_size =
                    min (bytes_to_read, static_cast<int>(data_.length() - offset_));
            memcpy(data_out, data_.c_str() + offset_, transfer_size);
            offset_ += transfer_size;

            bytes_read = transfer_size;
            has_data = true;
        }

        return has_data;
    }

private:
    IMPLEMENT_REFCOUNTING (MyResourceHandler)
    IMPLEMENT_LOCKING (MyResourceHandler)
};

#endif /* CEFUI_RESOURCE_HANDLER_H */
