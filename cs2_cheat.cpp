#include <Windows.h>
#include <d3d9.h>
#include <cmath>
#include <random>
#include <string>

// Placeholder for CS2 memory offsets (must be found via Cheat Engine/IDA Pro)
namespace offsets {
    constexpr uintptr_t localPlayer = 0xDEADBEEF; // offset
    constexpr uintptr_t entityList = 0xDEADBEEF;
    constexpr uintptr_t viewMatrix = 0xDEADBEEF;
    constexpr uintptr_t position = 0x134;
    constexpr uintptr_t headPosition = 0x140;
    constexpr uintptr_t viewAngles = 0x180;
    constexpr uintptr_t crosshairId = 0x1A0;
    constexpr uintptr_t health = 0x100;
    constexpr uintptr_t team = 0x104;
    constexpr uintptr_t flashAlpha = 0x1C0;
    constexpr uintptr_t smokeMaterial = 0xDEADBEEF;
}

// Configurable settings
namespace config {
    bool aimbotEnabled = true;
    bool ragebotEnabled = false;
    bool triggerbotEnabled = true;
    bool espEnabled = true;
    bool smokeFlashRemoverEnabled = true;
    float aimbotFov = 30.0f; // Degrees
    float aimbotSmoothness = 0.2f; // 0.0 (instant) to 1.0 (very smooth)
}

// Vector and matrix structs
struct Vector3 {
    float x, y, z;
};

struct Vector2 {
    float x, y;
};

struct Matrix4x4 {
    float m[4][4];
};

// Global variables
HANDLE g_hProcess = nullptr;
uintptr_t g_moduleBase = 0;
HWND g_hwnd = nullptr;
LPDIRECT3D9 g_pD3D = nullptr;
LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;

// Utility functions
template<typename T>
T ReadMemory(uintptr_t address) {
    T value;
    ReadProcessMemory(g_hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}

template<typename T>
void WriteMemory(uintptr_t address, T value) {
    WriteProcessMemory(g_hProcess, (LPVOID)address, &value, sizeof(T), nullptr);
}

bool IsValidEnemy(uintptr_t entity) {
    if (!entity) return false;
    int health = ReadMemory<int>(entity + offsets::health);
    int team = ReadMemory<int>(entity + offsets::team);
    int localTeam = ReadMemory<int>(ReadMemory<uintptr_t>(g_moduleBase + offsets::localPlayer) + offsets::team);
    return health > 0 && team != localTeam;
}

Vector3 CalcAngle(Vector3 src, Vector3 dst) {
    Vector3 angles;
    Vector3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    float hyp = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    angles.x = std::atan2(-delta.z, hyp) * 180.0f / 3.14159f;
    angles.y = std::atan2(delta.y, delta.x) * 180.0f / 3.14159f;
    angles.z = 0.0f;
    return angles;
}

bool WorldToScreen(Vector3 pos, Matrix4x4 matrix, Vector2& screen) {
    float w = matrix.m[3][0] * pos.x + matrix.m[3][1] * pos.y + matrix.m[3][2] * pos.z + matrix.m[3][3];
    if (w < 0.01f) return false;
    screen.x = (matrix.m[0][0] * pos.x + matrix.m[0][1] * pos.y + matrix.m[0][2] * pos.z + matrix.m[0][3]) / w;
    screen.y = (matrix.m[1][0] * pos.x + matrix.m[1][1] * pos.y + matrix.m[1][2] * pos.z + matrix.m[1][3]) / w;
    screen.x = (screen.x + 1.0f) * GetSystemMetrics(SM_CXSCREEN) / 2.0f;
    screen.y = (1.0f - screen.y) * GetSystemMetrics(SM_CYSCREEN) / 2.0f;
    return true;
}

void SimulateMouseClick() {
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(1); // Randomize delay slightly
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

// DirectX rendering for ESP
void DrawBox(Vector2 pos, const char* text, int health) {
    if (!g_pd3dDevice) return;
    // Simplified drawing (use D3DX9 or custom rendering library for real implementation)
    // Draw rectangle at pos.x, pos.y with health-based color
}

// Cheat functions
void Aimbot() {
    if (!config::aimbotEnabled && !config::ragebotEnabled) return;
    uintptr_t localPlayer = ReadMemory<uintptr_t>(g_moduleBase + offsets::localPlayer);
    Vector3 localPos = ReadMemory<Vector3>(localPlayer + offsets::position);
    Vector3 currentAngles = ReadMemory<Vector3>(localPlayer + offsets::viewAngles);
    float closestFov = config::aimbotFov;
    Vector3 targetAngles = currentAngles;

    for (int i = 0; i < 64; i++) { // Max 64 players
        uintptr_t entity = ReadMemory<uintptr_t>(g_moduleBase + offsets::entityList + i * 0x10);
        if (!IsValidEnemy(entity)) continue;
        Vector3 enemyHead = ReadMemory<Vector3>(entity + offsets::headPosition);
        Vector3 aimAngles = CalcAngle(localPos, enemyHead);
        float fov = std::hypot(aimAngles.x - currentAngles.x, aimAngles.y - currentAngles.y);
        if (fov < closestFov) {
            closestFov = fov;
            targetAngles = aimAngles;
        }
    }

    if (closestFov < config::aimbotFov) {
        if (config::ragebotEnabled) {
            WriteMemory<Vector3>(localPlayer + offsets::viewAngles, targetAngles);
            SimulateMouseClick(); // Auto-fire for ragebot
        } else {
            // Smooth aimbot
            Vector3 smoothedAngles;
            smoothedAngles.x = currentAngles.x + (targetAngles.x - currentAngles.x) * config::aimbotSmoothness;
            smoothedAngles.y = currentAngles.y + (targetAngles.y - currentAngles.y) * config::aimbotSmoothness;
            smoothedAngles.z = 0.0f;
            WriteMemory<Vector3>(localPlayer + offsets::viewAngles, smoothedAngles);
        }
    }
}

void Triggerbot() {
    if (!config::triggerbotEnabled) return;
    uintptr_t localPlayer = ReadMemory<uintptr_t>(g_moduleBase + offsets::localPlayer);
    int crosshairId = ReadMemory<int>(localPlayer + offsets::crosshairId);
    if (crosshairId > 0 && crosshairId <= 64) {
        uintptr_t entity = ReadMemory<uintptr_t>(g_moduleBase + offsets::entityList + (crosshairId - 1) * 0x10);
        if (IsValidEnemy(entity)) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(5, 10);
            Sleep(dis(gen)); // Random delay
            SimulateMouseClick();
        }
    }
}

void ESP() {
    if (!config::espEnabled || !g_pd3dDevice) return;
    Matrix4x4 viewMatrix = ReadMemory<Matrix4x4>(g_moduleBase + offsets::viewMatrix);
    for (int i = 0; i < 64; i++) {
        uintptr_t entity = ReadMemory<uintptr_t>(g_moduleBase + offsets::entityList + i * 0x10);
        if (!IsValidEnemy(entity)) continue;
        Vector3 pos = ReadMemory<Vector3>(entity + offsets::position);
        Vector2 screenPos;
        if (WorldToScreen(pos, viewMatrix, screenPos)) {
            int health = ReadMemory<int>(entity + offsets::health);
            DrawBox(screenPos, "Enemy", health);
        }
    }
}

void SmokeFlashRemover() {
    if (!config::smokeFlashRemoverEnabled) return;
    uintptr_t localPlayer = ReadMemory<uintptr_t>(g_moduleBase + offsets::localPlayer);
    float flashAlpha = ReadMemory<float>(localPlayer + offsets::flashAlpha);
    if (flashAlpha > 0.0f) {
        WriteMemory<float>(localPlayer + offsets::flashAlpha, 0.0f);
    }
    // Smoke removal (simplified, requires material offset)
    uintptr_t smokeMaterial = ReadMemory<uintptr_t>(g_moduleBase + offsets::smokeMaterial);
    if (smokeMaterial) {
        WriteMemory<int>(smokeMaterial + 0x10, 0); // Set opacity to 0
    }
}

// DirectX initialization (simplified)
bool InitDirectX() {
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) return false;
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_hwnd = FindWindowA(nullptr, "Counter-Strike 2");
    if (!g_hwnd) return false;
    HRESULT result = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);
    return SUCCEEDED(result);
}

// Main cheat thread
DWORD WINAPI CheatMain(LPVOID lpParam) {
    // Get CS2 process and module base
    g_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    g_moduleBase = 0x400000; // Placeholder, find via GetModuleHandle or pattern scanning
    InitDirectX();

    // Main loop
    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 1) { // Toggle cheats with INSERT
            config::aimbotEnabled = !config::aimbotEnabled;
            config::ragebotEnabled = !config::ragebotEnabled;
            config::triggerbotEnabled = !config::triggerbotEnabled;
            config::espEnabled = !config::espEnabled;
            config::smokeFlashRemoverEnabled = !config::smokeFlashRemoverEnabled;
        }
        Aimbot();
        Triggerbot();
        ESP();
        SmokeFlashRemover();
        Sleep(10); // Avoid high CPU usage
    }

    // Cleanup
    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (g_pD3D) g_pD3D->Release();
    CloseHandle(g_hProcess);
    return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, CheatMain, nullptr, 0, nullptr);
    }
    return TRUE;
}