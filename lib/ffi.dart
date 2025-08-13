import 'dart:ffi';
import 'dart:isolate';
import 'dart:typed_data';
import 'dart:math' as math;
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

  int estimatedJpegSize = width * height;
  final outputBuffer = malloc.allocate<Uint8>(estimatedJpegSize);

  int segBoundarySize = calculateOptimalBoundarySize(width, height);

  final segBoundary = malloc.allocate<Int32>(segBoundarySize);
  final segBoundarySizePtr = malloc.allocate<Int32>(1);

  final imageffi = dylib.lookupFunction<
      Void Function(
        Pointer<Uint8>,
        Pointer<Uint8>,
        Pointer<Uint8>,
        Int,
        Int,
        Int,
        Int,
        Pointer<Uint8>,
        Int,
        Pointer<Int32>,
        Pointer<Int32>,
      ),
      void Function(
        Pointer<Uint8>,
        Pointer<Uint8>,
        Pointer<Uint8>,
        int,
        int,
        int,
        int,
        Pointer<Uint8>,
        int,
        Pointer<Int32>,
        Pointer<Int32>,
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
      segBoundarySizePtr,
    );

    final actualResultSize = segBoundarySizePtr[0];

    final safeResultSize = math.min(actualResultSize, segBoundarySize);

    final processedImage = outputBuffer.asTypedList(estimatedJpegSize);

    final result = segBoundary.asTypedList(safeResultSize);

    port.send({
      "result": result,
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "image": processedImage,
    });
  } catch (e) {
    port.send({
      "error": e.toString(),
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "bufferSize": segBoundarySize,
    });
  } finally {
    malloc.free(yData);
    malloc.free(uData);
    malloc.free(vData);
    malloc.free(outputBuffer);
    malloc.free(segBoundary);
    malloc.free(segBoundarySizePtr);
  }
}

int calculateOptimalBoundarySize(int width, int height) {
  int pixelBasedEstimate = (width * height) ~/ 20;

  int perimeterBasedEstimate = (width + height) * 4;

  int objectDetectionEstimate = 100 * 4;

  int totalPixels = width * height;
  int resolutionBasedEstimate;

  if (totalPixels < 100000) {
    resolutionBasedEstimate = 2000;
  } else if (totalPixels < 500000) {
    resolutionBasedEstimate = 5000;
  } else if (totalPixels < 2000000) {
    resolutionBasedEstimate = 15000;
  } else {
    resolutionBasedEstimate = totalPixels ~/ 100;
  }

  int baseEstimate = [
    pixelBasedEstimate,
    perimeterBasedEstimate,
    objectDetectionEstimate,
    resolutionBasedEstimate,
  ].reduce(math.max);

  int estimateWithMargin = (baseEstimate * 1.5).round();

  int minSize = 1000;
  int maxSize = totalPixels;

  int finalSize = math.max(minSize, math.min(estimateWithMargin, maxSize));

  return finalSize;
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

    final result2 = segBoundary.asTypedList(segBoundarySize.value);

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
