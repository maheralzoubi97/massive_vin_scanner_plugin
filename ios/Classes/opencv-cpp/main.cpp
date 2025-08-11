
extern "C" __attribute__((visibility("default"))) __attribute__((used)) void image_ffi_path(char *path, int *objectCnt);
extern "C" __attribute__((visibility("default"))) __attribute__((used)) void image_ffi(unsigned char *yData, unsigned char *uData, unsigned char *vData,
                                                                                       int width, int height, int uvRowStride, int uvPixelStride,
                                                                                       unsigned char *buf, int bufSize,
                                                                                       int *segBoundary, int *segBoundarySize);
extern "C" __attribute__((visibility("default"))) __attribute__((used)) void initYolo8(char *modelPath, char *paramPath);
extern "C" __attribute__((visibility("default"))) __attribute__((used)) void disposeYolo8();

#include "main-seq.cpp"