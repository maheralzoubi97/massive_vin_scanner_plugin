import 'dart:ffi';
import 'dart:isolate';

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

// process image real time ...
processImageRealTime(Map<String, dynamic> data) {
  var port = data['send_port'] as SendPort;
  var cameraImage = data['cameraImage'] as CameraImage;
  var index = data['index'];
  var isolateTimeStamp = data['isolateTimeStamp'];

  int s = cameraImage.planes[0].bytes.length;

  final p = malloc.allocate<Uint8>(3 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);

  final segBoundary =
      malloc.allocate<Int32>(cameraImage.planes[0].bytes.length);
  final segBoundarySize = malloc.allocate<Int32>(1);

  final imageffi = dylib.lookupFunction<
      Void Function(Pointer<Uint8>, Int, Pointer<Int32>, Pointer<Int32>),
      void Function(
        Pointer<Uint8>,
        int,
        Pointer<Int32>,
        Pointer<Int32>,
      )>('image_ffi');

  try {
    imageffi(
      p,
      s,
      segBoundary,
      segBoundarySize,
    );
    final image = p.asTypedList(s); // return image ...
    final result2 =
        segBoundary.asTypedList(segBoundarySize[0]); // return segBoundary ...

    port.send({
      "result": result2,
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "image": image
    });
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
  }
}

processImageYuv24RealTime(Map<String, dynamic> data) {
  var port = data['send_port'] as SendPort;
  var cameraImage = data['cameraImage'] as CameraImage;
  var index = data['index'];
  var isolateTimeStamp = data['isolateTimeStamp'];

  int s = cameraImage.planes[0].bytes.length;

  final p = malloc.allocate<Uint8>(3 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);

  final segBoundary =
      malloc.allocate<Int32>(cameraImage.planes[0].bytes.length);
  final segBoundarySize = malloc.allocate<Int32>(1);

  final imageffi = dylib.lookupFunction<
      Void Function(
          Pointer<Uint8>, Int, Int, Int, Pointer<Int32>, Pointer<Int32>),
      void Function(
        Pointer<Uint8>,
        int,
        int,
        int,
        Pointer<Int32>,
        Pointer<Int32>,
      )>('image_ffi_yuv24');

  try {
    imageffi(
      p,
      s,
      cameraImage.width,
      cameraImage.height,
      segBoundary,
      segBoundarySize,
    );
    final image = p.asTypedList(s); // return image ...
    final result2 =
        segBoundary.asTypedList(segBoundarySize[0]); // return segBoundary ...

    port.send({
      "result": result2,
      "index": index,
      "isolateTimeStamp": isolateTimeStamp,
      "image": image
    });
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
  }
}

processImageWithWorker(SendPort port, CameraImage cameraImage, int index) {
  int s = cameraImage.planes[0].bytes.length;

  final p = malloc.allocate<Uint8>(3 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);

  final segBoundary =
      malloc.allocate<Int32>(cameraImage.planes[0].bytes.length);
  final segBoundarySize = malloc.allocate<Int32>(1);

  final imageffi = dylib.lookupFunction<
      Void Function(Pointer<Uint8>, Int, Pointer<Int32>, Pointer<Int32>),
      void Function(
        Pointer<Uint8>,
        int,
        Pointer<Int32>,
        Pointer<Int32>,
      )>('image_ffi');

  try {
    imageffi(
      p,
      s,
      segBoundary,
      segBoundarySize,
    );
    final image = p.asTypedList(s); // return image ...
    final result2 =
        segBoundary.asTypedList(segBoundarySize[0]); // return segBoundary ...

    port.send({
      "result": result2,
      "index": index,
      "image": image,
    });
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
  }
}

processImageRealTimeWithCore(Map<String, dynamic> data) {
  var cameraImage = data['cameraImage'] as CameraImage;
  var index = data['index'];

  int s = cameraImage.planes[0].bytes.length;

  final p = malloc.allocate<Uint8>(3 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);

  final segBoundary =
      malloc.allocate<Int32>(cameraImage.planes[0].bytes.length);
  final segBoundarySize = malloc.allocate<Int32>(1);

  final imageffi = dylib.lookupFunction<
      Void Function(Pointer<Uint8>, Int, Pointer<Int32>, Pointer<Int32>),
      void Function(
        Pointer<Uint8>,
        int,
        Pointer<Int32>,
        Pointer<Int32>,
      )>('image_ffi');

  try {
    imageffi(
      p,
      s,
      segBoundary,
      segBoundarySize,
    );
    final image = p.asTypedList(s); // return image ...
    final result2 =
        segBoundary.asTypedList(segBoundarySize[0]); // return segBoundary ...

    return {
      "result": result2,
      "index": index,
      "image": image,
    };
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
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

processImageRealTimeWithIsolateManager(Map<String, dynamic> data) async {
  var cameraImage = data['cameraImage'] as CameraImage;
  int s = cameraImage.planes[0].bytes.length;

  final p = malloc.allocate<Uint8>(3 * cameraImage.height * cameraImage.width);
  p.asTypedList(s).setRange(0, s, cameraImage.planes[0].bytes);

  final segBoundary =
      malloc.allocate<Int32>(cameraImage.planes[0].bytes.length);
  final segBoundarySize = malloc.allocate<Int32>(1);

  try {
    final imageffi = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Int, Pointer<Int32>, Pointer<Int32>),
        void Function(
          Pointer<Uint8>,
          int,
          Pointer<Int32>,
          Pointer<Int32>,
        )>('image_ffi');

    imageffi(
      p,
      s,
      segBoundary,
      segBoundarySize,
    );

    final image = p.asTypedList(s); // return image ...
    final result2 =
        segBoundary.asTypedList(segBoundarySize[0]); // return segBoundary ...

    return {
      "result": result2,
      "image": image,
    };
  } finally {
    malloc.free(p);
    malloc.free(segBoundary);
    malloc.free(segBoundarySize);
  }
}
