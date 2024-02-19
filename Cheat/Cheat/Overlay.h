#pragma once
#include "Function.h"
#include <chrono>
#include <thread>

#define GLFW_EXPOSE_NATIVE_WIN32   //�������������ʹ��  GLFW Native ����
#include <GLFW/glfw3.h> 
#include <GLFW/glfw3native.h>
#define GL_SILENCE_DEPRECATION

//�������Ի�����ʱ
#define TIME_NOW std::chrono::high_resolution_clock::now()
#define TIME_DURATION std::chrono::duration<double,std::milli>
//auto code_start = TIME_NOW;
//TIME_DURATION code_duration = TIME_NOW - code_start;
//cout << code_duration.count() << endl;


//������ɫ
#define RED     IM_COL32(255, 0, 0, 255)
#define GREEN   IM_COL32(0, 255, 0, 255)
#define BLUE    IM_COL32(0, 0, 255, 255)
#define CYAN    IM_COL32(0, 255, 255, 255)
#define YELLOW  IM_COL32(255, 255, 0, 255)
#define PURPLE  IM_COL32(128, 0, 128, 255)
#define ORANGE  IM_COL32(255, 165, 0, 255)
#define PINK    IM_COL32(255, 192, 203, 255)
#define GRAY    IM_COL32(128, 128, 128, 255)
#define WHITE   IM_COL32(255, 255, 255, 255)
#define BLACK   IM_COL32(10, 10, 10, 255)

//��������
bool OverlayInit();
void MainLoop();
void OverlayCleanup();


void DrawOverlay();
void DrawMenu();
void DrawRadarPanel();
void DrawEntity(GameEntity& entity);
void DrawPlayer(GamePlayer& player);
void DrawPlayerOnRadar(GamePlayer& player, ImU32 color);

static void glfw_error_callback(int error, const char* description) {
    printf("GLFW Error %d: %s\n", error, description);
}

bool OverlayInit() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330 core";
    
    glfwDefaultWindowHints();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    //glfwWindowHint(GLFW_REFRESH_RATE, 60);
    //glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);

    string wndTitle = RandomString(15);
    // Create window with graphics context
    GLFWwindow* glfw_wnd = glfwCreateWindow(GameVars.wndWidth, GameVars.wndHeight, wndTitle.c_str(), NULL, NULL);
    if (glfw_wnd == NULL) {
        return false;
    }
    
    glfwMakeContextCurrent(glfw_wnd);
    glfwSwapInterval(1); // Enable vsync  ��ֱͬ��

    //��ȡ���� HWND ���
    HWND hwnd = glfwGetWin32Window(glfw_wnd);
    if (hwnd == NULL) {
        return false;
    }

    //�����ڼ��� WS_EX_LAYERED ������ʵ�ֲ�����괩͸����͸�����������Ե��
    //����ټ��� WS_EX_TRANSPARENT �����ͻ���ȫ��͸����
    DWORD style = GetWindowLongW(hwnd, GWL_EXSTYLE);
    style |= WS_EX_LAYERED;
    //style |= WS_EX_TRANSPARENT;
    SetWindowLongW(hwnd, GWL_EXSTYLE, style);
    
    //���ú�ɫ(0,0,0)Ϊ͸��ɫ�����Ǻ�ɫ�����ش���Ϊ��ȫ��͸����255��
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

    //���洰�ھ��
    GameVars.glfw_wnd = glfw_wnd;
    GameVars.hwnd = hwnd;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;  //������imgui.ini

    //�����ڵ���ʽ��Ϊ������ɫ��������ʾΪ͸��
    ImGui::StyleColorsDark();
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.29f, 0.48f, 0.80f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(glfw_wnd, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    //io.Fonts->AddFontFromFileTTF("D:\\font\\simhei.ttf", 24.0f);
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    if (font == NULL) {
        return false;
    }
    
    return true;
}


void UpdateWindowStyle(bool fullyPassthrough) {
    DWORD style = GetWindowLongW(GameVars.hwnd, GWL_EXSTYLE);
    if (fullyPassthrough) {
        style |= WS_EX_TRANSPARENT;
    }
    else {
        style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongW(GameVars.hwnd, GWL_EXSTYLE, style);
}

void MainLoop()
{
    while (!glfwWindowShouldClose(GameVars.glfw_wnd)) {

        if (GameVars.wndChanged) {
            GameVars.wndChanged = false;
            GameVariables& v = GameVars;
            MoveWindow(v.hwnd, v.wndPosX, v.wndPosY, v.wndWidth, v.wndHeight, false);
        }

        if (CFG.show_menu_changed) {
            CFG.show_menu_changed = false;
            CFG.show_menu = !CFG.show_menu;
            UpdateWindowStyle(!CFG.show_menu); //�����ʾ�˲˵���Ҫ���Ĵ�����ʽ�������ܲ����˵�
        }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawOverlay();

        if (CFG.show_menu) {
            DrawMenu();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, GameVars.wndWidth, GameVars.wndHeight);

        //�����ɫ����Ϊ���ڣ��ͻ���ʾΪ͸��
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(GameVars.glfw_wnd);
    }
}


void OverlayCleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(GameVars.glfw_wnd);
    glfwTerminate();
}



//���־�����ʾ
void SetTextCenter(ImVec2& pos, const char* str)
{
    ImVec2 textSize = ImGui::CalcTextSize(str);
    pos.x = pos.x - textSize.x * 0.5f;
    pos.y = pos.y - textSize.y * 0.5f; 
}

void DrawOverlay()
{
    static string wndName = RandomString(10);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)GameVars.wndWidth, (float)GameVars.wndHeight));
    ImGui::Begin(wndName.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
    
    Memory.ClearCache();

    //��������ӽ�����
    GameVars.camera = Read<FCameraCacheEntry>(GameVars.local_player_camera_manager + Offsets.CameraCachePrivate);

    if (CFG.drawRadar) {
        DrawRadarPanel();
    }
    if (CFG.drawFOV) {
        ImGui::GetWindowDrawList()->AddCircle(ImVec2(GameVars.wndCenterX, GameVars.wndCenterY), CFG.aimbotFOV, GRAY);
    }

    vector<GamePlayer>& players = *GameVars.players;
    vector<GameEntity>& entities = *GameVars.entities;

    for (GameEntity& e : entities) {
        DrawEntity(e);
    }

    for (GamePlayer& p : players) {
        DrawPlayer(p);
    }

    ImGui::End();
}

void DrawMenu() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 400), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin(u8"Insert����  Home�˳�"); 

    if (ImGui::CollapsingHeader(u8"͸��",ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(3, NULL, false);
        ImGui::Checkbox(u8"��ʾ����", &CFG.espSkeleton);
        ImGui::Checkbox(u8"��ʾ����", &CFG.espDist);
        ImGui::NextColumn();

        ImGui::Checkbox(u8"��ʾѪ��", &CFG.espHealthBar);
        static float tmpMaxDist = CFG.maxDistance / 100;
        ImGui::Text(u8"��Զ����:"); ImGui::SameLine(); ImGui::SetNextItemWidth(70);
        ImGui::DragFloat(u8"##maxDist", &tmpMaxDist, 0.2f, 1, 200, "%.0f");
        CFG.maxDistance = tmpMaxDist * 100;
        ImGui::NextColumn();

        ImGui::Checkbox(u8"��ʾ����", &CFG.espBox);
        if (CFG.espBox) {
            ImGui::RadioButton(u8"ȫ��", &CFG.boxType, 0); ImGui::SameLine();
            ImGui::RadioButton(u8"�ǿ�", &CFG.boxType, 1);
        }
        ImGui::Columns(1);
    }
    ImGui::Spacing(); 

    if (ImGui::CollapsingHeader(u8"����", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(3, NULL, false);
        ImGui::Checkbox(u8"��������", &CFG.aimbotActive);
        ImGui::NextColumn();

        ImGui::Text(u8"���鷶Χ:"); ImGui::SameLine(); ImGui::SetNextItemWidth(70);
        ImGui::DragFloat(u8"##aimbotFOV", &CFG.aimbotFOV, 0.2f, 10, 1000, "%.0f");
        ImGui::NextColumn();

        ImGui::Checkbox(u8"��ʾ��Χ", &CFG.drawFOV);
        ImGui::Columns(1);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader(u8"�״�", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(3, NULL, false);
        ImGui::Checkbox(u8"��ʾ�״�", &CFG.drawRadar);

        static float tmpRadarDist = CFG.radarDist / 100;
        ImGui::Text(u8"��Զ����:"); ImGui::SameLine(); ImGui::SetNextItemWidth(70);
        ImGui::DragFloat(u8"##radarDist", &tmpRadarDist, 0.2f, 1, 200, "%.0f"); 
        CFG.radarDist = tmpRadarDist * 100;
        ImGui::NextColumn();

        ImGui::Text(u8"����xy:"); ImGui::SameLine();  
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(u8"##radarX", &CFG.radarCenter.x, 1.f, 0, (float)GameVars.wndWidth, "%.0f"); ImGui::SameLine();
        
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat(u8"##radarY", &CFG.radarCenter.y, 1.f, 0, (float)GameVars.wndHeight, "%.0f");
        ImGui::Text(u8"Բ����:"); ImGui::SameLine(); ImGui::SetNextItemWidth(70);
        ImGui::DragInt(u8"##radarCircleCount", &CFG.radarCircleCount, 0.1f, 0, 20);
        ImGui::NextColumn();

        ImGui::Text(u8"�뾶:"); ImGui::SameLine(); ImGui::SetNextItemWidth(70);
        ImGui::DragFloat(u8"##radarRadius", &CFG.radarRadius, 1.f, 20, 500, "%.0f");
        ImGui::Columns(1);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader(u8"DEBUG", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(3, NULL, false);
        ImGui::Checkbox(u8"��ʾ����Actor", &CFG.drawActorAll);
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Overlay FPS: %.0f", io.Framerate);
        ImGui::NextColumn();
        
        ImGui::Checkbox(u8"��ʾ����Index", &CFG.drawBoneIndex);
        ImGui::Columns(1);

    }
    ImGui::Spacing();

    ImGui::End();
}


void DrawEntity(GameEntity& entity)
{
    Vector3 worldPos = GetTransform(entity.rootComponent).translation;
    Vector3 scrPos = WorldToScreen(worldPos);
    float distance = GetDistance(worldPos);

    //ȷ����������Ұ�У�����С����Զ͸�Ӿ���
    if (scrPos.z > 0.f && distance <= CFG.maxDistance
        && scrPos.x >= 0.f && scrPos.x <= (float)GameVars.wndWidth
        && scrPos.y >= 0.f && scrPos.y <= (float)GameVars.wndHeight)
    {
        //cout << entity.displayName << ": X= " << scrPos.x << ", Y= " << scrPos.y << ", Z= " << scrPos.z << endl;
        string text = entity.displayName + "[" + std::to_string((int32)distance) + "]";
        const char* charArr = text.c_str();
        ImVec2 textPos = ImVec2(scrPos.x, scrPos.y);
        SetTextCenter(textPos, charArr);

        ImGui::GetWindowDrawList()->AddText(textPos, YELLOW, charArr);
    }
    
}


//���ƹ������index������ʹ��
void DrawBoneIndex(GamePlayer& player)
{
    ULONG_PTR mesh = player.mesh;
    //cout << "Mesh " << hex << mesh << endl;

    int32 currentRead = Read<int32>(mesh + Offsets.MeshToCurrentRead);

    FTransform compToWorld = Read<FTransform>(mesh + Offsets.ComponentToWorld);
    FMatrix matrix = compToWorld.ToMatrixWithScale();

    TArray boneTransArr = Read<TArray>(mesh + (currentRead * 0x10) + Offsets.MeshToTransformsArray);
    for (uint32 i = 0; i < boneTransArr.count; i++)
    {
        FTransform boneTrans = Read<FTransform>(boneTransArr.addr + (i * sizeof(FTransform)));
        FMatrix boneMatrix = boneTrans.ToMatrixWithScale();
        FMatrix world = MatrixMultiplication(boneMatrix, matrix);
        Vector3 worldPos = { world._41,world._42,world._43 };
        Vector3 pos = WorldToScreen(worldPos);

        string tName = to_string(i);
        const char* dName = tName.c_str();
        ImVec2 textPos = ImVec2(pos.x, pos.y);
        SetTextCenter(textPos, dName);

        ImGui::GetWindowDrawList()->AddText(textPos, CYAN, dName);
    }

    
}

Vector3 GetBonePos(ULONG_PTR mesh, int32 index)
{
    int32 currentRead = Read<int32>(mesh + Offsets.MeshToCurrentRead);
    FTransform compToWorld = Read<FTransform>(mesh + Offsets.ComponentToWorld);
    FMatrix matrix = compToWorld.ToMatrixWithScale();

    TArray boneTransArr = Read<TArray>(mesh + (currentRead * 0x10) + Offsets.MeshToTransformsArray);

    FTransform boneTrans = Read<FTransform>(boneTransArr.addr + (index * sizeof(FTransform)));
    FMatrix boneMatrix = boneTrans.ToMatrixWithScale();
    FMatrix world = MatrixMultiplication(boneMatrix, matrix);
    Vector3 worldPos = { world._41,world._42,world._43 };

    return worldPos;
}

//���ƹ���͸��
void DrawSkeleton(GamePlayer& player, ImU32 color)
{
    vector<int32>& indexArr = player.boneIndexArray;
    ImVec2 previous;

    for (size_t i = 0; i < indexArr.size(); i++) {
        int32 index = indexArr[i];
        if (index == -1) {
            continue;
        }
        Vector3 worldPos = GetBonePos(player.mesh, index);
        Vector3 scrPos = WorldToScreen(worldPos);
        ImVec2 imPos = ImVec2(scrPos.x, scrPos.y);

        if (i > 0 && indexArr[i - 1] != -1) {
            ImGui::GetWindowDrawList()->AddLine(previous, imPos, color);
        }
        previous = imPos;
    }
    
}


void DrawHealthBar(float x, float y, float barW, float barH, float percentage, ImU32 color) {
    float border = 2;
    float filledWidth = (barW - (border * 2)) * percentage;

    ImU32 EMPTY = IM_COL32(10, 10, 10, 220);

    //�ϱ߿���1
    ImVec2 top_brd_l(x, y);
    ImVec2 top_brd_r(x + barW, y);
    ImVec2 top_bar_l(x + border, y + 1);
    ImVec2 top_bar_r(x + barW - border, y + 1);
    ImVec2 btm_bar(x + border + filledWidth, y + barH - border);
    ImVec2 btm_brd_l(x, y + barH);
    ImVec2 btm_brd_r(x + barW, y + barH - border);

    auto drawList = ImGui::GetWindowDrawList();
    //�߿�
    drawList->AddLine(top_brd_l, top_brd_r, BLACK);
    drawList->AddRectFilled(btm_brd_l, btm_brd_r, BLACK);
    drawList->AddRectFilled(top_bar_l, btm_brd_l, BLACK);
    drawList->AddRectFilled(top_bar_r, btm_brd_r, BLACK);

    //Ѫ��
    drawList->AddRectFilled(top_bar_l, btm_bar, color);
    drawList->AddRectFilled(top_bar_r, btm_bar, EMPTY);
}

void DrawBox(float x, float y, float w, float h, ImU32 color, bool corneredBox) {
    auto drawList = ImGui::GetWindowDrawList();
    if (corneredBox) {
        float lineW = w / 3;
        float lineH = h / 4;
        ImVec2 p1(x, y);
        ImVec2 p2(x + w, y);
        ImVec2 p3(x, y + h);
        ImVec2 p4(x + w, y + h);

        drawList->AddLine(p1, ImVec2(p1.x + lineW, p1.y), color);
        drawList->AddLine(p1, ImVec2(p1.x, p1.y + lineH), color);

        drawList->AddLine(p2, ImVec2(p2.x - lineW, p2.y), color);
        drawList->AddLine(p2, ImVec2(p2.x, p2.y + lineH), color);

        drawList->AddLine(p3, ImVec2(p3.x, p3.y - lineH), color);
        drawList->AddLine(p3, ImVec2(p3.x + lineW, p3.y), color);

        drawList->AddLine(p4, ImVec2(p4.x - lineW, p4.y), color);
        drawList->AddLine(p4, ImVec2(p4.x, p4.y - lineH), color);
    }
    else {
        ImVec2 boxP1(x, y);
        ImVec2 boxP2(x + w, y + h);
        drawList->AddRect(boxP1, boxP2, color);
    }
}

void DrawESP(GamePlayer& player, ImU32 ESP_Color, ImU32 healthColor) {
    if (CFG.espSkeleton) {
        DrawSkeleton(player, ESP_Color);
    }

    int32 root_index = 0;
    int32 head_index = player.boneInfo["Head"];
    int32 spine_index = player.boneInfo["spine_3"];

    ULONG_PTR mesh = player.mesh;
    Vector3 head = GetBonePos(mesh, head_index);
    Vector3 root = GetBonePos(mesh, root_index);
    Vector3 spine = GetBonePos(mesh, spine_index);

    head.z += 20;
    Vector3 top = WorldToScreen(head);
    Vector3 bottom = WorldToScreen(root);
    Vector3 middle = WorldToScreen(spine);

    float boxHeight = abs(top.y - bottom.y);
    float boxWidth = boxHeight * 0.45f;
    float half_W = boxWidth / 2;

    EPlayerHealthStatus status = player.healthStatus;
    if (CFG.espBox && status != HS_Downed && 
        status != HS_Incapacitated && status != HS_Dead) 
    {
        DrawBox((middle.x - half_W), top.y, boxWidth, boxHeight, ESP_Color, CFG.boxType == 1);
    }
    if (CFG.espHealthBar && healthColor != 0 && boxWidth > 50) {
        float health = player.health;
        float max = player.maxHealth;
        if (health >= 0.f && max > 0.f && health <= max) {
            float barH = 10;
            float x = middle.x - half_W;
            float y = top.y - barH - 5;
            DrawHealthBar(x, y, boxWidth, barH, (health / max), healthColor);
        }
    }

    string text = player.displayName;
    text += "(" + GetStatusString(player.healthStatus) + ")";
    
    if (CFG.espDist) {
        float dist = GetDistance(root) / 100;
        text += " [" + std::to_string((int32)dist) + "]";
    }

    const char* charArr = text.c_str();
    ImVec2 size = ImGui::CalcTextSize(charArr);
    ImVec2 textPos(middle.x - (size.x / 2), bottom.y);

    ImGui::GetWindowDrawList()->AddText(textPos, ESP_Color, charArr);
    
}

void DrawPlayer(GamePlayer& player) {
    ImU32 healthColor = 0;
    ImU32 radarColor = GRAY;
    ImU32 ESP_Color = GRAY;

    if (player.type == CyberneticsSwat_V2_C) {
        healthColor = IM_COL32(80, 25, 230, 255);//blue
        radarColor = WHITE;
        ESP_Color = CYAN;
    }
    else if (player.type == CyberneticsSuspect_V2_C) {
        healthColor = IM_COL32(181, 27, 25, 255);   //red
        radarColor = RED;
        ESP_Color = RED;
        if (IsVisible(player.mesh)) {
            ESP_Color = YELLOW;
        }
    }

    UpdatePlayerStatus(player);
    if (CFG.drawRadar) {
        DrawPlayerOnRadar(player, radarColor);
    }

    Vector3 worldPos = GetTransform(player.rootComponent).translation;
    Vector3 scrPos = WorldToScreen(worldPos);
    float distance = GetDistance(worldPos);

    if (scrPos.z > 0.f && distance <= CFG.maxDistance) {
        if (CFG.drawBoneIndex) {
            DrawBoneIndex(player);
            return;
        }
        DrawESP(player, ESP_Color, healthColor);
    }
}



//�����״ﱳ��
void DrawRadarPanel()
{
    ImVec2& center = CFG.radarCenter;
    float radius = CFG.radarRadius;
    int32 circleCount = CFG.radarCircleCount;

    //����ɫ��ǰ��ɫ
    ImU32 BG = IM_COL32(1, 1, 1, 150);
    ImU32 darkGreen = IM_COL32(0, 150, 0, 200);
    ImU32 brightGreen = IM_COL32(0, 250, 0, 200);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddCircleFilled(center, radius, BG); //����

    if (circleCount > 0)
    {
        float spacing = radius / circleCount; // ����ͬ��Բ֮��ļ��
        for (int32 i = 1; i <= circleCount; ++i) {
            if (i == circleCount) {
                drawList->AddCircle(center, spacing * i, darkGreen, 0, 2.f); // �������Բ�Ӵ�
                continue;
            }
            drawList->AddCircle(center, spacing * i, darkGreen); // ����ͬ��Բ
        }
    }
    drawList->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), brightGreen); // ˮƽ��
    drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), brightGreen); // ��ֱ��

    //�����״����ļ�ͷ����ͷ��������������ɣ�
    ImVec2 top, bottom, left, right;
    top = bottom = left = right = center;

    top.y -= 10;
    bottom.y += 4;
    left.x -= 8;
    left.y += 10;
    right.x += 8;
    right.y += 10;

    drawList->AddTriangleFilled(top, bottom, left, darkGreen);
    drawList->AddTriangleFilled(top, right, bottom, darkGreen);
}

//���״��ϻ���һ��������ҵ�λ��
void DrawPlayerOnRadar(GamePlayer& player, ImU32 color) {
    ImVec2& center = CFG.radarCenter;
    float radius = CFG.radarRadius;
    float radarDist = CFG.radarDist;

    Vector3 myPos = GameVars.camera.POV.Location;
    float myYaw = GameVars.camera.POV.Rotation.y;
    //cout << "myPos x=" << myPos.x << ", y=" << myPos.y << ", z=" << myPos.z << endl;

    FTransform transform = GetTransform(player.rootComponent);
    Vector3 playerPos = transform.translation;

    float dX = playerPos.x - myPos.x;
    float dY = playerPos.y - myPos.y;

    float dist = (float)sqrt(dX * dX + dY * dY);
    if (dist > radarDist) {
        return;
    }

    //�����еľ���ת��Ϊ��Ļ���ؾ���
    dist = dist / radarDist * radius;

    //�������������ĽǶ�
    float angleRadian = (float)atan2(dX, dY);
    float angleDegree = ToDegree(angleRadian);

    //���ǵ������ Yaw
    angleDegree = AnglesAdd(angleDegree, myYaw);

    //���������Ҿ����״����ĵ��ƫ�ƣ�Ȼ��õ���������
    angleRadian = ToRadian(angleDegree);
    float offsetCos = dist * cos(angleRadian);
    float offsetSin = dist * sin(angleRadian);
    ImVec2 finalPos = ImVec2(center.x + offsetCos, center.y - offsetSin);

    //�������ʶ�����������״��ϻ�һ�� X
    EPlayerHealthStatus status = player.healthStatus;
    if (status == HS_Incapacitated || status == HS_Dead) {
        ImVec2 p = finalPos;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x - 5, p.y - 5), ImVec2(p.x + 5, p.y + 5), color);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x + 5, p.y - 5), ImVec2(p.x - 5, p.y + 5), color);
        return;
    }

    ImGui::GetWindowDrawList()->AddCircleFilled(finalPos, 5.0f, color);
    
    //���������ҵĳ���
    float playerYaw = transform.rot.ToEulerAngles().y; // -180 �� 180

    //�������������������
    playerYaw = AnglesAdd(playerYaw, myYaw * -1.0f);

    dist = 20;//���ɫ����ķ������ 20 ���ص���
    angleRadian = ToRadian(playerYaw);
    offsetCos = dist * cos(angleRadian);
    offsetSin = dist * sin(angleRadian);
    ImVec2 faceTo = ImVec2(finalPos.x + offsetSin, finalPos.y - offsetCos);

    ImGui::GetWindowDrawList()->AddLine(finalPos, faceTo, color);
}


