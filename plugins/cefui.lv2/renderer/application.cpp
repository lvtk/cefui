// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <iostream>
#include <sstream>

#include "include/capi/cef_app_capi.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"

#include "common/util.h"
#include "renderer/render_app.h"
#include "renderer/application.h"

class Application::Priv
{
public:
    Priv (Application* owner)
    {
        app = new RenderApp();
    }

    ~Priv()
    {
        app = nullptr;
    }

    CefRefPtr<RenderApp> get_app() const { return app; }

private:
    CefRefPtr<RenderApp> app;

};

Application::Application()
{
    priv.reset (new Priv (this));
}

Application::~Application()
{
    priv.reset (nullptr);
}

int Application::exec_subprocess (int argc, char** argv) const
{
    const CefMainArgs args (argc, argv);
    return CefExecuteProcess (args, priv->get_app().get(), 0);
}
