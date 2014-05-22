/*
    simple_handler.h - This file is part of cefui
    Copyright (C) 2013  Michael Fisher <mfisher31@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef CEFUI_CLIENT_CONTROLLER_H
#define CEFUI_CLIENT_CONTROLLER_H

#include <iostream>
#include <list>
#include <memory>
#include <set>

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

class ClientController
{
public:

    class Listener
    {
    public:
        Listener() { }
        virtual ~Listener() { }
        virtual void client_received_control (const std::string& port, const double value) =0;
    };

    ClientController();
    ~ClientController();

    void add_listener (Listener*);
    void remove_listener (Listener*);
    void close_all_browsers (bool force_close);
    bool is_closing() const;
    int create_browser (void* widget, const std::string& url=std::string());

private:
    class Impl;
    std::unique_ptr<Impl> impl;
    friend class ClientHandler;
    friend class Impl;
};

#endif  /* CEFUI_CLIENT_CONTROLLER_H */
