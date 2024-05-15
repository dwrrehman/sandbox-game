
#include "raylib.h"




int main(void) {
    InitWindow(800, 450, "dwrr");

    Texture2D texture = LoadTexture("./blocks.png");

    Model models[2] = {0};
    models[0] = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));
    models[1] = LoadModelFromMesh(GenMeshCustom());

    for (int i = 0; i < 2; i++) models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    Camera camera = { { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    int currentModel = 0;
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            
        }
	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode3D(camera);
	DrawModel(models[currentModel], position, 1.0f, WHITE);
	DrawGrid(10, 1.0);
	EndMode3D();
	DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
	DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
	DrawText("MOUSE LEFT BUTTON to CYCLE PROCEDURAL MODELS", 40, 410, 10, BLUE);
	EndDrawing();
    }
    UnloadTexture(texture); // Unload texture
    for (int i = 0; i < 2; i++) UnloadModel(models[i]);
    CloseWindow(); 
}

