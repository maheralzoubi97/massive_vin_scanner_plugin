import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'massive_vin_scanner_platform_interface.dart';

/// An implementation of [MassiveVinScannerPlatform] that uses method channels.
class MethodChannelMassiveVinScanner extends MassiveVinScannerPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('massive_vin_scanner');

  @override
  Future<String?> getPlatformVersion() async {
    final version =
        await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
