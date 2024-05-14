// C raylib client for sandbox block game.
// written by dwrr on 202405131.211411:

#define RCAMERA_STANDALONE
#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iso646.h>

typedef uint64_t nat; 
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

static const int screen_width = 800;
static const int screen_height = 450;


/*
int main(void) {

	InitWindow(screen_width, screen_width, "dwrr's sandbox block game");

	Mesh mesh = {0};

	mesh.triangleCount = 1;
	mesh.vertexCount = mesh.triangleCount * 3;
	mesh.vertices = (float *) MemAlloc(mesh.vertexCount * 3 * sizeof(float));  
	mesh.texcoords = (float *) MemAlloc(mesh.vertexCount * 2 * sizeof(float)); 
	mesh.normals = (float *) MemAlloc(mesh.vertexCount * 3 * sizeof(float)); 

	mesh.vertices[0] = 0;
	mesh.vertices[1] = 0;
	mesh.vertices[2] = 0;
	mesh.normals[0] = 0;
	mesh.normals[1] = 1;
	mesh.normals[2] = 0;
	mesh.texcoords[0] = 0;
	mesh.texcoords[1] = 0;

	mesh.vertices[3] = 1;
	mesh.vertices[4] = 0;
	mesh.vertices[5] = 2;
	mesh.normals[3] = 0;
	mesh.normals[4] = 1;
	mesh.normals[5] = 0;
	mesh.texcoords[2] = 0.5f;
	mesh.texcoords[3] = 1.0f;

	mesh.vertices[6] = 2;
	mesh.vertices[7] = 0;
	mesh.vertices[8] = 0;
	mesh.normals[6] = 0;
	mesh.normals[7] = 1;
	mesh.normals[8] = 0;
	mesh.texcoords[4] = 1;
	mesh.texcoords[5] = 0;

	UploadMesh(&mesh, false);


	Image checked = GenImageChecked(3, 3, 1, 1, RED, GREEN);
	Texture2D texture = LoadTextureFromImage(checked);
	UnloadImage(checked);

	Model models[1] = {0};
	models[0] = LoadModelFromMesh(mesh);
	models[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

	Camera camera = { 
		{ 5.0f, 5.0f, 5.0f }, 
		{ 0.0f, 0.0f, 0.0f }, 
		{ 0.0f, 1.0f, 0.0f }, 
		45.0f, 
		0 
	};

	Vector3 position = { 0.0f, 0.0f, 0.0f };
 
	SetTargetFPS(120);
    
	while (not WindowShouldClose()) {
        	UpdateCamera(&camera, CAMERA_ORBITAL);
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			
		}

		if (IsKeyDown(KEY_A)) position.x -= 0.1f;
		else if (IsKeyDown(KEY_F)) position.x += 0.1f;

		BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode3D(camera);
		DrawModel(models[0], position, 1.0f, WHITE);
		DrawGrid(10, 1.0);
		EndMode3D();

		DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
		DrawText("test of the block game...", 40, 410, 10, BLACK);
		DrawText("custom mesh", 580, 10, 20, DARKBLUE); 
		EndDrawing();
	}

    	UnloadTexture(texture);
	UnloadModel(models[0]);
	CloseWindow();
}
*/


int main(void) {
	SetRandomSeed(42);

	float heights[20] = {0};
	Vector3 positions[20] = {0};
	Color colors[20] = {0};

	for (int i = 0; i < 20; i++) {
		heights[i] = (float)GetRandomValue(1, 12);
		positions[i] = (Vector3){ (float)GetRandomValue(-15, 15), heights[i]/2.0f, (float)GetRandomValue(-15, 15) };
		colors[i] = (Color){ GetRandomValue(20, 255), GetRandomValue(10, 55), 30, 255 };
	}

	InitWindow(screen_width, screen_width, "dwrr's sandbox block game");

	Camera camera = {0};
	camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };
	camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	DisableCursor();
	SetTargetFPS(120);

	while (true) {


		get

		if (IsKeyPressed(KEY_BACKSLASH)) break; 
		if (IsKeyPressed(KEY_COMMA)) TakeScreenshot("screenshot.png");
		if (IsKeyPressed(KEY_PERIOD)) OpenURL("https://google.com");

		if (IsKeyDown(KEY_W)) 		camera.position.x += 0.1;
		if (IsKeyDown(KEY_S)) 		camera.position.x -= 0.1;
		if (IsKeyDown(KEY_A)) 		camera.position.y += 0.1;
		if (IsKeyDown(KEY_D)) 		camera.position.y -= 0.1;
		if (IsKeyDown(KEY_LEFT_SHIFT))	camera.position.z += 0.1;
		if (IsKeyDown(KEY_SPACE)) 	  	camera.position.z -= 0.1;

		BeginDrawing();
		ClearBackground(RAYWHITE);

		//****************************************************************************
		BeginMode3D(camera);
		DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); 
		DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);
		DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);
		DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);
		for (int i = 0; i < 20; i++) {
			DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
			DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
		}
		DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(5, 5, 330, 100, BLUE);
		DrawRectangle(600, 5, 195, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(600, 5, 195, 100, BLUE);
		EndMode3D();
		//****************************************************************************


		DrawText("first person camera test", 15, 15, 10, BLACK);
		
		DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", 
				camera.position.x, camera.position.y, camera.position.z), 
			610, 60, 10, BLACK
		);
		DrawText(TextFormat("- forward: (%06.3f, %06.3f, %06.3f)", 
				camera.target.x, camera.target.y, camera.target.z), 
			610, 75, 10, BLACK
		);
		DrawText(TextFormat("- up: (%06.3f, %06.3f, %06.3f)", 
			camera.up.x, camera.up.y, camera.up.z), 
			610, 90, 10, BLACK
		);

		EndDrawing();
	}
	CloseWindow(); 
}







/*




































static int window_width = 1600;
static int window_height = 1000;
static float aspect = 1.6f;

static const float fovy = 1.22173f /*radians*/;
static const float znear = 0.01f;
static const float zfar = 1000.0f;

static const float camera_sensitivity = 0.005f;
static const float camera_accel = 0.00003f;
static const float drag = 0.95f;

static const int32_t ms_delay_per_frame = 8;


static const char* vertex_shader_code = "        			\n\
#version 120	  	     						\n\
									\n\
attribute vec3 position;                                		\n\
attribute vec2 input_uv;                               			\n\
									\n\
varying vec2 output_uv;							\n\
uniform mat4 matrix;							\n\
                                          				\n\
void main() {                                				\n\
	gl_Position = matrix * vec4(position, 1.0);              	\n\
	output_uv = input_uv;                           		\n\
}                                                       		\n";

static const char* fragment_shader_code = "				\n\
#version 120								\n\
									\n\
varying vec2 output_uv;							\n\
uniform sampler2D atlas_texture;					\n\
									\n\
void main() {								\n\
//	gl_FragColor.rbg = texture2D(atlas_texture, output_uv).rbg;	\n\
	gl_FragColor.rbg = vec3(output_uv, 0.0);			\n\
	gl_FragColor.a = 1.0;						\n\
}									\n";


// 
// sampler2D uTexture
// void main() {
// 	vec4 textureColor = texture(uTexture, aTexCoords);
//
//}


struct vec3 {float x,y,z;};

typedef float* mat4;

static inline void perspective(mat4 result, float fov, float asp, float zNear, float zFar) {
	const float t = tanf(fov / 2.0f);
	result[4 * 0 + 0] = 1.0f / (asp * t);
	result[4 * 1 + 1] = 1.0f / t;
	result[4 * 2 + 2] = -(zFar + zNear) / (zFar - zNear);
	result[4 * 2 + 3] = -1.0f;
	result[4 * 3 + 2] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

static inline float inversesqrt(float y) {
	float x2 = y * 0.5f;
	int32_t i = *(int32_t *)&y;
	i = 0x5f3759df - (i >> 1); 	// glm uses a86 for last three digits.
	y = *(float*) &i;
	return y * (1.5f - x2 * y * y);
}

static inline struct vec3 normalize(struct vec3 v) {
   float s = inversesqrt(v.x * v.x + v.y * v.y + v.z * v.z);
   return (struct vec3) {v.x * s, v.y * s, v.z * s};
}

static inline struct vec3 cross(struct vec3 x, struct vec3 y) {
	return (struct vec3) {
		x.y * y.z - y.y * x.z,
		x.z * y.x - y.z * x.x,
		x.x * y.y - y.x * x.y
	};
}

static inline float dot(struct vec3 a, struct vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline void look_at(mat4 result, struct vec3 eye, struct vec3 f, struct vec3 up) {
	 //TODO: is this neccessary?
	struct vec3 s = normalize(cross(f, up));
	struct vec3 u = cross(s, f);

	result[4 * 0 + 0] =  s.x;
	result[4 * 1 + 0] =  s.y;
	result[4 * 2 + 0] =  s.z;
	result[4 * 0 + 1] =  u.x;
	result[4 * 1 + 1] =  u.y;
	result[4 * 2 + 1] =  u.z;
	result[4 * 0 + 2] = -f.x;
	result[4 * 1 + 2] = -f.y;
	result[4 * 2 + 2] = -f.z;
	result[4 * 3 + 0] = -dot(s, eye);
	result[4 * 3 + 1] = -dot(u, eye);
	result[4 * 3 + 2] =  dot(f, eye);
	result[4 * 3 + 3] =  1;
}

static inline void multiply_matrix(mat4 out, mat4 A, mat4 B) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out[4 * i + j] = 
		A[4 * i + 0] * B[4 * 0 + j] + 
		A[4 * i + 1] * B[4 * 1 + j] + 
		A[4 * i + 2] * B[4 * 2 + j] + 
		A[4 * i + 3] * B[4 * 3 + j];
        }
    }
}

static	bool debug = false;
static	bool quit = false;
static	bool tab = false;
static 	bool should_move_camera = true;
static 	bool is_fullscreen = false;
static	int counter = 0;
static	float delta = 0.0;

static	float pitch = 0.0f, yaw = 0.0f;
static	struct vec3 position = {10, 5, 10};
static	struct vec3 velocity = {0, 0, 0};

static	struct vec3 forward = 	{0, 0, -1};
static	struct vec3 straight = 	{0, 0, 1};
static	struct vec3 up = 	{0, 1, 0};
static	struct vec3 right = 	{-1, 0, 0};

static inline void move_camera(void) {
	const float pi_over_2 = 1.57079632679f;
	if (pitch > pi_over_2) pitch = pi_over_2 - 0.0001f;
	else if (pitch < -pi_over_2) pitch = -pi_over_2 + 0.0001f;

	forward.x = -sinf(yaw) * cosf(pitch);
	forward.y = -sinf(pitch);
	forward.z = -cosf(yaw) * cosf(pitch);
	forward = normalize(forward);

	right.x = -cosf(yaw);
	right.y = 0.0;
	right.z = sinf(yaw);
	right = normalize(right);
	
	straight = cross(right, up);
}


























void UpdateCamera(Camera *camera, int mode) {

    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = true;
    bool rotateAroundTarget = false;
    bool lockView = true;
    bool rotateUp = false;

        if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) CameraPitch(camera, CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_Q)) CameraRoll(camera, -CAMERA_ROTATION_SPEED);
        if (IsKeyDown(KEY_E)) CameraRoll(camera, CAMERA_ROTATION_SPEED);


            CameraYaw(camera, -mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

        // Keyboard support
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_D)) CameraMoveRight(camera, CAMERA_MOVE_SPEED, moveInWorldPlane);

}

*/


