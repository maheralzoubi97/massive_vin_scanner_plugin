import 'dart:ffi';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:camera/camera.dart';
import 'package:ffi/ffi.dart';
import 'package:image_picker/image_picker.dart';
import 'package:massive_vin_scanner/function/function.dart';

// initial seqmentation model ...

initialModel(modelPath, modelParam) async {
  final tempModelPath = (await copy(modelPath)).toNativeUtf8();
  final tempParamPath = (await copy(modelParam)).toNativeUtf8();

  final model = dylib.lookupFunction<
      Void Function(Pointer<Utf8>, Pointer<Utf8>),
      void Function(
        Pointer<Utf8>,
        Pointer<Utf8>,
      )>('initYolo8');

  model(tempModelPath, tempParamPath);
  Pointer<Utf8> s = malloc.allocate(1);
  logger.e("initial Segmentation Model DONE");
}

// close seqmentation model ...

closeModel() {
  dylib.lookupFunction<
      Void Function(Pointer<Utf8>, Pointer<Utf8>),
      void Function(
        Pointer<Utf8>,
        Pointer<Utf8>,
      )>('disposeYolo8');

  logger.e("close Segmentation Model Done");
}

processImageRealTime(Map<String, dynamic> data) {
  var port = data['send_port'] as SendPort;
  var cameraImage = data['cameraImage'] as CameraImage;
  var index = data['index'];
  var isolateTimeStamp = data['isolateTimeStamp'];

  // Get image dimensions
  int width = cameraImage.width;
  int height = cameraImage.height;

  // Get YUV420 plane data
  Uint8List yPlane = cameraImage.planes[0].bytes;
  Uint8List uPlane = cameraImage.planes[1].bytes;
  Uint8List vPlane = cameraImage.planes[2].bytes;

  // Get stride information
  int uvRowStride = cameraImage.planes[1].bytesPerRow;
  int uvPixelStride = cameraImage.planes[1].bytesPerPixel ?? 1;

  // Allocate memory for YUV planes
  final yData = malloc.allocate<Uint8>(yPlane.length);
  final uData = malloc.allocate<Uint8>(uPlane.length);
  final vData = malloc.allocate<Uint8>(vPlane.length);

  // Copy YUV data to allocated memory
  yData.asTypedList(yPlane.length).setRange(0, yPlane.length, yPlane);
  uData.asTypedList(uPlane.length).setRange(0, uPlane.length, uPlane);
  vData.asTypedList(vPlane.length).setRange(0, vPlane.length, vPlane);

  // Allocate buffer for output JPEG image (estimate size)
  int estimatedJpegSize = width * height; // Rough estimate
  final outputBuffer = malloc.allocate<Uint8>(estimatedJpegSize);

  // Allocate memory for detection results
  final segBoundary = malloc.allocate<Int32>(1000); // Adjust size as needed
  final segBoundarySize = malloc.allocate<Int32>(1);

  // Define the FFI function signature for YUV420
  final imageffi = dylib.lookupFunction<
      Void Function(
        Pointer<Uint8>, // yData
        Pointer<Uint8>, // uData
        Pointer<Uint8>, // vData
        Int, // width
        Int, // height
        Int, // uvRowStride
        Int, // uvPixelStride
        Pointer<Uint8>, // output buffer
        Int, // buffer size
        Pointer<Int32>, // segBoundary
        Pointer<Int32>, // segBoundarySize
      ),
      void Function(
        Pointer<Uint8>, // yData
        Pointer<Uint8>, // uData
        Pointer<Uint8>, // vData
        int, // width
        int, // height
        int, // uvRowStride
        int, // uvPixelStride
        Pointer<Uint8>, // output buffer
        int, // buffer size
        Pointer<Int32>, // segBoundary
        Pointer<Int32>, // segBoundarySize
      )>('image_ffi');

  try {
    imageffi(
      yData,
      uData,
      vData,
      width,
      height,
      uvRowStride,
      uvPixelStride,
      outputBuffer,
      estimatedJpegSize,
      segBoundary,
      segBoundarySize,
    );

    // Get the processed JPEG image
    final processedImage = outputBuffer.asTypedList(estimatedJpegSize);

    // Get the detection results
    final result = segBoundary.asTypedList(segBoundarySize[0]);

    port.send({
      "result": result,
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "image": processedImage,
    });
  } finally {
    // Clean up allocated memory
    malloc.free(yData);
    malloc.free(uData);
    malloc.free(vData);
    malloc.free(outputBuffer);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
  }
}

void processImageBgra8888RealTime(Map<String, dynamic> data) {
  final port = data['send_port'] as SendPort;
  final cameraImage =
      data['cameraImage'] as CameraImage; // Ensure this handles BGRA8888 data
  final index = data['index'];
  final isolateTimeStamp = data['isolateTimeStamp'];

  final s = cameraImage.planes[0].bytes.length;
  final p = malloc.allocate<Uint8>(4 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);
  final segBoundary =
      malloc.allocate<Int32>(cameraImage.height * cameraImage.width);
  final segBoundarySize = malloc.allocate<Int32>(1);
  final jpegBuf = malloc.allocate<Pointer<Uint8>>(1);
  final jpegSize = malloc.allocate<Int32>(1);

  // Lookup the FFI function for BGRA8888 image processing
  final imageFfi = dylib.lookupFunction<
      Void Function(Pointer<Uint8>, Int32, Int32, Int32, Pointer<Int32>,
          Pointer<Int32>, Pointer<Pointer<Uint8>>, Pointer<Int32>),
      void Function(
          Pointer<Uint8>,
          int,
          int,
          int,
          Pointer<Int32>,
          Pointer<Int32>,
          Pointer<Pointer<Uint8>>,
          Pointer<Int32>)>('image_ffi_bgra8888');

  try {
    imageFfi(
      p,
      s,
      cameraImage.width,
      cameraImage.height,
      segBoundary,
      segBoundarySize,
      jpegBuf,
      jpegSize,
    );
    final imageBytes = jpegBuf.value.asTypedList(jpegSize.value);

    final result2 = segBoundary
        .asTypedList(segBoundarySize.value); // Return segment boundaries

    port.send({
      "result": result2,
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "image": imageBytes
    });
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
    malloc.free(jpegBuf.value);
    malloc.free(jpegBuf);
    malloc.free(jpegSize);
  }
}

final ImagePicker _picker = ImagePicker();

processImageByPicker(String type) async {
  final imageFile = await _picker.pickImage(
      source: type == "gallery" ? ImageSource.gallery : ImageSource.camera);
  var imagePath = imageFile?.path.toNativeUtf8() ?? "none".toNativeUtf8();
  final imageFFI = dylib.lookupFunction<
      Void Function(Pointer<Utf8>, Pointer<Int>),
      void Function(Pointer<Utf8>, Pointer<Int>)>('image_ffi_path');
  Pointer<Uint32> s = malloc.allocate(1);
  imageFFI(
    imagePath,
    s as Pointer<Int>,
  );
  malloc.free(s);

  return imagePath.toDartString();
}
