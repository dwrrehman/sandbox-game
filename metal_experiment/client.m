//
// cc sdl-metal-example.m `sdl2-config --cflags --libs` -framework Metal -framework QuartzCore && ./a.out
//
#include <SDL.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

int main(int argc, const char** argv) {
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
	SDL_InitSubSystem(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("universe game", -1, -1, 640, 480, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	const CAMetalLayer *swapchain = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(renderer);
	const id<MTLDevice> gpu = swapchain.device;
	const id<MTLCommandQueue> queue = [gpu newCommandQueue];

	MTLClearColor color = MTLClearColorMake(0, 0, 0, 1);
	bool quit = false;
	SDL_Event e;

	while (!quit) {
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) { quit = true; break; }

        }

        @autoreleasepool {
            id<CAMetalDrawable> surface = [swapchain nextDrawable];

            color.red = (color.red > 1.0) ? 0 : color.red + 0.01;

            MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
            pass.colorAttachments[0].clearColor = color;
            pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
            pass.colorAttachments[0].storeAction = MTLStoreActionStore;
            pass.colorAttachments[0].texture = surface.texture;

            id<MTLCommandBuffer> buffer = [queue commandBuffer];
            id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
            [encoder endEncoding];
            [buffer presentDrawable:surface];
            [buffer commit];
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}



