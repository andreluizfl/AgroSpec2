#include "modulemanager.h"
#include "spectrometermodule.h"

namespace Agrospec {

ModuleManager::ModuleManager()
{

}

void ModuleManager::InitializeGUIModules()
{
    SpectrometerModule::Initialize();
}

void ModuleManager::InitializeConsoleModules()
{
    SpectrometerModule::Initialize();
}

void ModuleManager::ReleaseGUIModules()
{
    SpectrometerModule::Release();
}

void ModuleManager::ReleaseConsoleModules()
{
    SpectrometerModule::Release();
}

}
