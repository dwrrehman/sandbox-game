

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>

#include "cmt/cmt.h"
#include "cmt/types_metal.h"

int main(void) {

	MtDevice                   *device;
	MtCommandQueue             *cmdQueue;
	MtRenderPipelineDescriptor *pipDesc;
	MtLibrary                  *lib;
	MtFunction                 *vertFunc, *fragFunc;
	MtRenderPipelineState      *pip;

	device   = mtCreateDevice();
	lib      = mtDefaultLibrary(device);
	cmdQueue = mtCommandQueueCreate(device);
	pipDesc  = mtRenderDescCreate(MtPixelFormatBGRA8Unorm);

	vertFunc = mtCreateFunc(lib, "vertexShader");
	fragFunc = mtCreateFunc(lib, "fragmentShader");

	mtSetFunc(pipDesc, vertFunc, MT_FUNC_VERT);
	mtSetFunc(pipDesc, fragFunc, MT_FUNC_FRAG);

	pip = mtRenderStateCreate(device, pipDesc);
}



