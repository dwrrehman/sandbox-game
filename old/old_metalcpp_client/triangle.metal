#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct my_vertex {
    vector_float2 position;
    vector_float4 color;
};

struct vertex_out {
    float4 position [[position]];
    float4 color;
};

vertex struct vertex_out vertexShader(uint vertexID [[vertex_id]],      /// instead, use:    struct my_vertex in [[ stage_in ]]
	constant struct my_vertex* vertices [[buffer(0)]],
	constant vector_uint2* viewportSizePointer [[buffer(1)]]
) {
	float2 pixelSpacePosition = vertices[vertexID].position.xy;
	vector_float2 viewportSize = vector_float2(*viewportSizePointer);
	struct vertex_out out;
	out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
	out.position.xy = pixelSpacePosition;// / (viewportSize / 2.0);
	out.color = vertices[vertexID].color;
	return out;
}

fragment float4 fragmentShader(struct vertex_out in [[stage_in]]) {
	return in.color;
}






































/*











#include <metal_stdlib>
#include <simd/simd.h>






struct vertex_in {
	float4 position [[attribute(0)]];
	float4 color [[attribute(1)]];
};


struct vertex_out {
	float4 position [[position]];
	float4 color;
};


vertex float4 vertexShader(const vertex_in vertexIn [[stage_in]]) {



    vector_float2 viewportSize = vector_float2(*viewportSizePointer);

    //// To convert from positions in pixel space to positions in clip-space,
    //
//  divide the pixel coordinates by half the size of the viewport.
    out.position = vector_float4(vertices[vertexID].position.xy, 1.0);
    //out.position.xy = pixelSpacePosition / (viewportSize / 2.0);

    out.color = vertices[vertexID].color;
    return out;


	return vertexIn.position;
}


















struct my_vertex {
    vector_float3 position;
    vector_float4 color;
};

using namespace metal;

// Vertex shader outputs and fragment shader inputs
struct vertex_out
{
    // The [[position]] attribute of this member indicates that this value
    // is the clip space position of the vertex when this structure is
    // returned from the vertex function.
    float4 position [[position]];

    // Since this member does not have a special attribute, the rasterizer
    // interpolates its value with the values of the other triangle vertices
    // and then passes the interpolated value to the fragment shader for each
    // fragment in the triangle.
    float4 color;
};

vertex struct vertex_out vertexShader(uint vertexID [[vertex_id]],
	constant struct my_vertex *vertices [[buffer(0)]],
	constant vector_uint2 *viewportSizePointer [[buffer(1)]]
) {
    RasterizerData out;

    // Index into the array of positions to get the current vertex.
    // The positions are specified in pixel dimensions (i.e. a value of 100
    // is 100 pixels from the origin).
    // float2 pixelSpacePosition = ;

    vector_float2 viewportSize = vector_float2(*viewportSizePointer);

    //// To convert from positions in pixel space to positions in clip-space,
    //
//  divide the pixel coordinates by half the size of the viewport.
    out.position = vector_float4(vertices[vertexID].position.xy, 1.0);
    //out.position.xy = pixelSpacePosition / (viewportSize / 2.0);

    out.color = vertices[vertexID].color;
    return out;
}

fragment float4 fragmentShader(struct vertex_out in [[stage_in]]) {
    return in.color;
}





*/




