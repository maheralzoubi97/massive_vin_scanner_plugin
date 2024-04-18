#ifndef FLUTTER_PLUGIN_MASSIVE_VIN_SCANNER_PLUGIN_H_
#define FLUTTER_PLUGIN_MASSIVE_VIN_SCANNER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace massive_vin_scanner {

class MassiveVinScannerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  MassiveVinScannerPlugin();

  virtual ~MassiveVinScannerPlugin();

  // Disallow copy and assign.
  MassiveVinScannerPlugin(const MassiveVinScannerPlugin&) = delete;
  MassiveVinScannerPlugin& operator=(const MassiveVinScannerPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace massive_vin_scanner

#endif  // FLUTTER_PLUGIN_MASSIVE_VIN_SCANNER_PLUGIN_H_
