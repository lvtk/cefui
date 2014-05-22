
#ifndef CEFUI_APPLICATION_H
#define CEFUI_APPLICATION_H

#include <memory>

class Application
{
public:

    Application();
    virtual ~Application();

    int exec_subprocess (int argc, char** argv) const;

private:
    class Priv; friend class Priv;
    std::unique_ptr<Priv> priv;

};

#endif
