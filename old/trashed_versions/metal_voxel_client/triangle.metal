#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct my_vertex {
    vector_float3 position;
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


	// delete all of this,  we don't need it:


	float2 pixelSpacePosition = vertices[vertexID].position.xy;
	vector_float2 viewportSize = vector_float2(*viewportSizePointer);
	struct vertex_out out;
	//out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
	//out.position.xy = pixelSpacePosition / (viewportSize / 2.0);

	out.position.x = (vertexID == 2) ? 3.0 : -1.0;
	out.position.y = (vertexID == 0) ? -3.0 : 1.0;
	out.position.zw = 1.0;

	out.color.r = (vertexID == 2) ? 1.0 : 0;
	out.color.b = (vertexID == 0) ? 0 : 1.0;
	out.color.ga = 1.0;

	return out;
}

fragment float4 fragmentShader(struct vertex_out in [[stage_in]]) {

	//in.color.b = 1.0;
	//in.color.g = 0.5;
	//in.color.r = 1.0;
	//in.color.a = 1.0;

	// this is where everything will happen!

	return in.color;
}






































/*












#version 420 core

uniform float  aspect;
uniform float  focal_length;
uniform mat4x4 tm_eye;

layout(location=0) in vec2 pos;

out smooth vec3 ray_pos;    // ray start position
out smooth vec3 ray_dir;    // ray start direction

void main(void)
{
    vec4 p;

    // perspective projection
    p           = tm_eye * vec4( pos.x/aspect, pos.y, 0.0, 1.0 );
    ray_pos     = p.xyz;
    p          -= tm_eye * vec4( 0.0, 0.0, -focal_length, 1.0 );
    ray_dir     = normalize( p.xyz );

    gl_Position = vec4( pos, 0.0, 1.0 );
}
















renderEncoder.setFragmentBytes(
  &params,
  length: MemoryLayout<Uniforms>.stride,
  index: 12)







fragment float4 fragment_main(
  constant Params &params [[buffer(12)]],
  VertexOut in [[stage_in]])

















#include <metal_stdlib>
using namespace metal;

struct Uniforms
{
    float4x4 modelViewProjectionMatrix;
};

struct Vertex
{
    packed_float4 position;
    packed_float2 texCoords;
};

struct ProjectedVertex
{
    float4 position [[position]];
    float2 texCoords [[user(texcoords)]];
};

vertex ProjectedVertex vertex_main(const device Vertex *vertices [[buffer(0)]],
                                   constant Uniforms *uniforms   [[buffer(1)]],
                                   uint vertexID [[vertex_id]])
{
    float4 position = vertices[vertexID].position;
    float2 texCoords = vertices[vertexID].texCoords;

    ProjectedVertex outVert;
    outVert.position = uniforms->modelViewProjectionMatrix * position;
    outVert.texCoords = texCoords;
    return outVert;
}

fragment half4 fragment_main(ProjectedVertex inVert [[stage_in]],
                             texture2d<float, access::sample> diffuseTexture [[texture(0)]],
                             sampler textureSampler [[sampler(0)]])
{
    float4 color = diffuseTexture.sample(textureSampler, inVert.texCoords);

    if (color.a < 0.5)
        discard_fragment();

    return half4(color);
}















#include <metal_stdlib>

using namespace metal;

typedef struct {
    float4 position [[position]];
} vertex_t;

vertex vertex_t vertex_function(const device vertex_t *vertices [[buffer(0)]], uint vid [[vertex_id]]) {
    return vertices[vid];
}

fragment half4 fragment_function(vertex_t interpolated [[stage_in]]) {
    float4 color = interpolated.position;
    color += 1.0; // move from range -1..1 to 0..2
    color *= 0.5; // scale from range 0..2 to 0..1
    return half4(color);
}























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





