#include "include/massive_vin_scanner/massive_vin_scanner_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "massive_vin_scanner_plugin.h"

void MassiveVinScannerPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  massive_vin_scanner::MassiveVinScannerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
