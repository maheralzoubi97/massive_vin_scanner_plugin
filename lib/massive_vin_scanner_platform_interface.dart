import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'massive_vin_scanner_method_channel.dart';

abstract class MassiveVinScannerPlatform extends PlatformInterface {
  /// Constructs a MassiveVinScannerPlatform.
  MassiveVinScannerPlatform() : super(token: _token);

  static final Object _token = Object();

  static MassiveVinScannerPlatform _instance = MethodChannelMassiveVinScanner();

  /// The default instance of [MassiveVinScannerPlatform] to use.
  ///
  /// Defaults to [MethodChannelMassiveVinScanner].
  static MassiveVinScannerPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [MassiveVinScannerPlatform] when
  /// they register themselves.
  static set instance(MassiveVinScannerPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
