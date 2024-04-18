
import 'massive_vin_scanner_platform_interface.dart';

class MassiveVinScanner {
  Future<String?> getPlatformVersion() {
    return MassiveVinScannerPlatform.instance.getPlatformVersion();
  }
}
