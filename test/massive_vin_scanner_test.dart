import 'package:flutter_test/flutter_test.dart';
import 'package:massive_vin_scanner/massive_vin_scanner.dart';
import 'package:massive_vin_scanner/massive_vin_scanner_platform_interface.dart';
import 'package:massive_vin_scanner/massive_vin_scanner_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockMassiveVinScannerPlatform
    with MockPlatformInterfaceMixin
    implements MassiveVinScannerPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final MassiveVinScannerPlatform initialPlatform = MassiveVinScannerPlatform.instance;

  test('$MethodChannelMassiveVinScanner is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelMassiveVinScanner>());
  });

  test('getPlatformVersion', () async {
    MassiveVinScanner massiveVinScannerPlugin = MassiveVinScanner();
    MockMassiveVinScannerPlatform fakePlatform = MockMassiveVinScannerPlatform();
    MassiveVinScannerPlatform.instance = fakePlatform;

    expect(await massiveVinScannerPlugin.getPlatformVersion(), '42');
  });
}
