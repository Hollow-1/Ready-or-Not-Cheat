#include "Overlay.h"


//ȫ�ְ������ӣ�������ʾ�����ز˵�
//HHOOK hook = nullptr;  


//��������
//LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
bool IsRunAsAdmin();
void GameCacheRoutine();
void AimbotThread();

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    char wndTitle[256];

    if (GetWindowTextA(hWnd, wndTitle, sizeof(wndTitle)) > 0) {
        if (wndTitle[0] != '\0') {
            std::cout << "Title:[" << wndTitle << "]\t\tHandle:" << hWnd << endl;
        }
    }
    return TRUE;
}

int main()
{
	SetConsoleOutputCP(CP_UTF8);//���ÿ���̨������ָ�ʽ
    //ShowWindow(GetConsoleWindow(), SW_HIDE);

    //������ж��㴰�ڱ���
    //EnumWindows(EnumWindowsProc, NULL);


    if (!IsRunAsAdmin()) {
        MessageBoxW(NULL, L"���Թ���ԱȨ�����У�", L"��ʾ", MB_OK);
        return 0;
    }
    /*
    //Set keyboard hook
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hook == NULL) {
        MessageBoxW(NULL, L"ע����̹���ʧ�ܣ�", L"��ʾ", MB_OK);
        return 1;
    }
    */
    /*
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    */
    
    while ((GameVars.game_hwnd = FindWindowW(NULL, GameVars.game_window_title)) == NULL) {
        cout << u8"û���ҵ���Ϸ����" << endl;
        Sleep(1000);
    }

    //��ʼ���ڴ��ȡ�ͻ���ģ��
    if (!Memory.Init()) {
        MessageBoxW(NULL, L"�ڴ�ģ���ʼ��ʧ�ܣ�", L"��ʾ", MB_OK);
        return 1;
    }
    if (!OverlayInit()) {
        MessageBoxW(NULL, L"����ģ���ʼ��ʧ�ܣ�", L"��ʾ", MB_OK);
        return 1;
    }

    //���������߳�
    CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GameCacheRoutine), NULL, NULL, NULL);
    Sleep(100);//�ȴ��������

    //���������߳�
    CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AimbotThread), NULL, NULL, NULL);
    
    MainLoop();

    OverlayCleanup();

    //UnhookWindowsHookEx(hook);

	return 0;
}

void PrintPtr(std::string text, uintptr_t ptr) {
    std::cout << text << ptr << std::endl;
}

void GameCacheRoutine()
{
	while (!GameVars.shouldExit)
	{
        if (GetAsyncKeyState(VK_INSERT)) {
            CFG.show_menu_changed = true;
        }

        //�������HOME����������Ϸ���ڹرգ��˳�����
        if (GetAsyncKeyState(VK_HOME) || !IsWindow(GameVars.game_hwnd)) {
            glfwSetWindowShouldClose(GameVars.glfw_wnd, GLFW_TRUE);
            GameVars.shouldExit = true;
            break;
        }
        RECT gameRect;
        GetClientRect(GameVars.game_hwnd, &gameRect);
        POINT scrPoint = { gameRect.left,gameRect.top };
        ClientToScreen(GameVars.game_hwnd, &scrPoint);
        //cout << scrPoint.x << "-" << scrPoint.y << "-" << gameRect.right << "-" << gameRect.bottom << endl;
        if (GameVars.wndPosX != scrPoint.x || GameVars.wndPosY != scrPoint.y ||
            GameVars.wndWidth != gameRect.right || GameVars.wndHeight != gameRect.bottom)
        {
            GameVars.wndPosX = scrPoint.x;
            GameVars.wndPosY = scrPoint.y;
            GameVars.wndWidth = gameRect.right;
            GameVars.wndHeight = gameRect.bottom;
            GameVars.wndCenterX = GameVars.wndWidth / 2.0f;
            GameVars.wndCenterY = GameVars.wndHeight / 2.0f;
            GameVars.wndChanged = true;
        }

		GameVars.g_world = Read<ULONG_PTR>(GameVars.moduleBase + Offsets.GWorld);
		GameVars.game_instance = Read<ULONG_PTR>(GameVars.g_world + Offsets.GameInstance);
		GameVars.local_player_array = Read<ULONG_PTR>(GameVars.game_instance + Offsets.LocalPlayers);
		GameVars.local_player = Read<ULONG_PTR>(GameVars.local_player_array);
		GameVars.local_player_controller = Read<ULONG_PTR>(GameVars.local_player + Offsets.PlayerController);
        GameVars.local_player_camera_manager = Read<ULONG_PTR>(GameVars.local_player_controller + Offsets.PlayerCameraManager);
		GameVars.local_player_pawn = Read<ULONG_PTR>(GameVars.local_player_controller + Offsets.ControllerToPawn);
		GameVars.local_player_root = Read<ULONG_PTR>(GameVars.local_player_pawn + Offsets.RootComponent);
		GameVars.local_player_state = Read<ULONG_PTR>(GameVars.local_player_pawn + Offsets.PlayerState);
		GameVars.persistent_level = Read<ULONG_PTR>(GameVars.g_world + Offsets.PersistentLevel);
		GameVars.actor_array = Read<TArray>(GameVars.persistent_level + Offsets.AActors);


        /*
		PrintPtr("uworld ", GameVars.g_world);
		PrintPtr("game instance ", GameVars.game_instance);
		PrintPtr("L Player Array ", GameVars.local_player_array);
		PrintPtr("L Player ", GameVars.local_player);
		PrintPtr("L Player Controller ", GameVars.local_player_controller);
        PrintPtr("L Player Camera Manager ", GameVars.local_player_camera_manager);
		PrintPtr("L Player Pawn ", GameVars.local_player_pawn);
		PrintPtr("L Player Root ", GameVars.local_player_root);
		PrintPtr("L Player State ", GameVars.local_player_state);
		PrintPtr("P Level ", GameVars.persistent_level);
		PrintPtr("Actors ", GameVars.actor_array.addr);
		PrintPtr("Actor Count ", GameVars.actor_array.count);
        printf("Resolution %d x %d\n",GameVars.wndWidth, GameVars.wndHeight);
        */
        
        //���֮ǰ�Ļ��棬Ȼ�����Actor�����浽 tmpPlayers �� tmpEntities
        (*GameVars.tmpPlayers).clear();
        (*GameVars.tmpEntities).clear();

		for (uint32 i = 0; i < GameVars.actor_array.count; i++)
		{
			ULONG_PTR pActor = Read<ULONG_PTR>(GameVars.actor_array.addr + (i * Offsets.pointerSize));
			if (pActor && pActor != GameVars.local_player_pawn)
			{
                string name = UObject::getFName(pActor).GetStringWithoutNumber();
                ActorFilter(name, pActor);
			}
		}

        //��������
        vector<GamePlayer>* tp = GameVars.players;
        vector<GameEntity>* te = GameVars.entities;

        GameVars.players = GameVars.tmpPlayers;
        GameVars.entities = GameVars.tmpEntities;

        GameVars.tmpPlayers = tp;
        GameVars.tmpEntities = te;

		Sleep(200);
	}
}

void MouseMoveTo(float x, float y)
{
    float center_x = GameVars.wndCenterX;
    float center_y = GameVars.wndCenterY;

    float target_x = x - center_x;
    float target_y = y - center_y;

    mouse_event(MOUSEEVENTF_MOVE, (DWORD)target_x, (DWORD)target_y, 0, 0);
}

void AimbotThread() {
    while (true) {
        bool isAimbotActive = CFG.aimbotActive && GetAsyncKeyState(VK_RBUTTON);
        if (isAimbotActive) {
            vector<GamePlayer>& players = *GameVars.players;
            Vector3 target;
            float targetDist = FLT_MAX;

            //Ѱ�ҵ���
            for (GamePlayer& p : players) {
                if (p.type != CyberneticsSuspect_V2_C ||
                    p.healthStatus == HS_Incapacitated ||
                    p.healthStatus == HS_Dead) 
                {
                    continue;
                }
                int32 bone_index = p.boneInfo["Head"];
                Vector3 bone = GetBonePos(p.mesh, bone_index);
                Vector3 bone_pos = WorldToScreen(bone);
                if (bone_pos.z < 0.f) {
                    continue;
                }
                float dist = CalcDistance(
                    GameVars.wndCenterX, GameVars.wndCenterY,
                    bone_pos.x, bone_pos.y
                );

                if (dist < CFG.aimbotFOV && dist < targetDist) {
                    targetDist = dist;
                    target = bone_pos;
                }
            }
            //����ҵ��ˣ����ƶ����
            if (targetDist < CFG.aimbotFOV) {
                MouseMoveTo(target.x, target.y);
            }
        }
        Sleep(5);
    }
}



/*
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbdData = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            int KeyCode = kbdData->vkCode;

            printf(u8"���룺 %d\n", KeyCode);
            if (KeyCode == VK_HOME) {
                CFG.show_menu = !CFG.show_menu;
            }
            if (KeyCode == VK_HOME) {
                GameVars.shouldExit = true;
                glfwSetWindowShouldClose(GameVars.glfw_wnd, GLFW_TRUE);
            }

        }
    }

    // ������һ�����ӣ�������ڵĻ�
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
*/

bool IsRunAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&SIDAuthNT, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &administratorsGroup))
    {
        if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin))
        {
            isAdmin = FALSE;
        }
        FreeSid(administratorsGroup);
    }

    return isAdmin == TRUE;
}