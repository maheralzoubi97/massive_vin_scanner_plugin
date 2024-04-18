
extern "C" __attribute__((visibility("default"))) __attribute__((used))
void image_ffi_path(char *path,int *objectCnt);
extern "C" __attribute__((visibility("default"))) __attribute__((used))
void image_ffi(unsigned char *buf,  int size , int *segBoundary , int *segBoundarySize);
extern "C" __attribute__((visibility("default"))) __attribute__((used))
void initYolo8(char *modelPath, char *paramPath);
extern "C" __attribute__((visibility("default"))) __attribute__((used))
void disposeYolo8();

#include "main-seq.cpp"