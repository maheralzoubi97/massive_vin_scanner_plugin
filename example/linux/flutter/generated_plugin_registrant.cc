//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <massive_vin_scanner/massive_vin_scanner_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) massive_vin_scanner_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "MassiveVinScannerPlugin");
  massive_vin_scanner_plugin_register_with_registrar(massive_vin_scanner_registrar);
}
