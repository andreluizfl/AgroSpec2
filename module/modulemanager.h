#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

namespace Agrospec {

class ModuleManager
{
public:
    static void InitializeGUIModules();
    static void InitializeConsoleModules();
    static void ReleaseGUIModules();
    static void ReleaseConsoleModules();
    ModuleManager();
};

}
#endif // MODULEMANAGER_H
