//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <file_selector_windows/file_selector_windows.h>
#include <massive_vin_scanner/massive_vin_scanner_plugin_c_api.h>

void RegisterPlugins(flutter::PluginRegistry* registry) {
  FileSelectorWindowsRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("FileSelectorWindows"));
  MassiveVinScannerPluginCApiRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("MassiveVinScannerPluginCApi"));
}
