

#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint ncnn_yolox_flutter.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'massive_vin_scanner'
  s.version          = '0.0.1'
  s.summary          = 'A new flutter plugin project.'
  s.description      = <<-DESC
  A new flutter plugin project.
                         DESC
  s.homepage         = 'https://github.com/AutomotionJo/massive_vin_scanner_plugin/'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'connectedmotion' }
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*'
  s.dependency 'Flutter'
  s.dependency 'OpenCV', '4.3.0'
  s.platform = :ios, '14'

  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386 arm64' }
  s.swift_version = '5.0'
  

  ## If you do not need to download the ncnn library, remove it. 
  ## From here
  #  s.prepare_command = <<-CMD
  #     rm -rf "ncnn.xcframework"
  #     rm -rf "openmp.xcframework"
  #     curl "https://github.com/KoheiKanagu/ncnn_yolox_flutter/releases/download/0.0.6/ncnn-ios-bitcode_xcframework.zip" -L -o "ncnn-ios-bitcode_xcframework.zip"
  #     unzip "ncnn-ios-bitcode_xcframework.zip"
  #     rm "ncnn-ios-bitcode_xcframework.zip"
  #   CMD
  ## Up to here

  # https://medium.com/flutter-community/integrating-c-library-in-a-flutter-app-using-dart-ffi-38a15e16bc14
  s.preserve_paths = 'ncnn.xcframework', 'openmp.xcframework'
  s.xcconfig = { 
    'OTHER_LDFLAGS' => '-framework ncnn -framework openmp',
    'OTHER_CFLAGS' => '-DUSE_NCNN_SIMPLEOCV -DMASSIVE_VIN_SCANNER_IOS',
  }
  s.ios.vendored_frameworks = 'ncnn.xcframework', 'openmp.xcframework'
  s.library = 'c++'
end




