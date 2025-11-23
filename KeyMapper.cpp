// 特殊键不需要单引号，使用 VK_ 开头的常量,在43,49,52,59,67,71,75,80行改
// 包含 windows.h 必须在定义 UNICODE 之后
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <wchar.h> // 用于 wcsncpy

// --- 全局变量和常量 ---
#define WM_TRAY_ICON_MESSAGE (WM_USER + 1)
#define ID_ENABLE_DISABLE 1001
#define ID_QUIT           1002
#define ID_TRAY_ICON      1003

// 托盘图标数据结构
NOTIFYICONDATA nid; 
bool is_active = true; // 功能激活状态
HHOOK hHook = NULL;    // 键盘钩子句柄

// 按键重复状态跟踪：解决 F6/F7 连续输出问题
bool is_f6_pressed = false;
bool is_f7_pressed = false;


// --- 函数声明 ---
void AddTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void UpdateTrayMenu(HWND hWnd);
void SendMappedKeys(WORD vKey);
void ToggleActiveState(HWND hWnd);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


// --- 实用函数 ---

// 最终版本：精确控制 F6 和 F7 序列，并在末尾添加 Shift 切换输入法模式。
void SendMappedKeys(WORD vKey) {
    
    // 不再在映射后切换输入法（保持当前 IME 状态）
    
    // --- 1. 根据按键发送对应字符 ---
    
    if (vKey == VK_F6) {//vk_k6改成自己的键
        // F6: 模拟物理按键 'y' 的按下与释放（不按 Shift），
        // 以便微软拼音等输入法将其作为拼音字母接收并进入候选组合流程。
        INPUT keyPress[2];
        ZeroMemory(keyPress, sizeof(keyPress));

        keyPress[0].type = INPUT_KEYBOARD;
        keyPress[0].ki.wVk = 'Y'; // 虚拟键码，对应键盘上的 Y 键（不按 Shift 则为小写 y）,在这更改

        keyPress[1].type = INPUT_KEYBOARD;
        keyPress[1].ki.wVk = 'Y';//在这更改
        keyPress[1].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(2, keyPress, sizeof(INPUT));
        Sleep(5);

    } else if (vKey == VK_F7) {//KV_F7改成自己的键
        // F7 -> '^' (必须 Shift + 6)
        
        INPUT shiftSix[4];
        ZeroMemory(shiftSix, sizeof(shiftSix));
        
        // 1) 按下 Shift 
        shiftSix[0].type = INPUT_KEYBOARD;
        shiftSix[0].ki.wVk = VK_SHIFT;//在这更改按键,最好是组合键的第一个

        // 2) 按下 6
        shiftSix[1].type = INPUT_KEYBOARD;
        shiftSix[1].ki.wVk = '6';//在这更改,第二个组合键

        // 3) 释放 6
        shiftSix[2].type = INPUT_KEYBOARD;
        shiftSix[2].ki.wVk = '6';//在这更改,释放第二个组合键
        shiftSix[2].ki.dwFlags = KEYEVENTF_KEYUP;
        
        // 4) 释放 Shift
        shiftSix[3].type = INPUT_KEYBOARD;
        shiftSix[3].ki.wVk = VK_SHIFT;//在这更改,释放第一个组合键
        shiftSix[3].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(4, shiftSix, sizeof(INPUT));
        Sleep(5);
    }
}
// 键盘钩子回调函数 (按键重复控制)
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
        WORD vkCode = pKey->vkCode;

        if (vkCode == VK_F6 || vkCode == VK_F7) {
            
            bool* is_pressed_flag = (vkCode == VK_F6) ? &is_f6_pressed : &is_f7_pressed;

            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // 仅在功能激活且首次按下时执行映射
                if (is_active && !*is_pressed_flag) {
                    *is_pressed_flag = true; // 标记为已按下
                    SendMappedKeys(vkCode);
                }
                // 屏蔽所有 F6/F7 按下事件 (包括首次和自动重复)
                return 1;
            } 
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                // 释放事件发生时，重置状态
                *is_pressed_flag = false;
                // 屏蔽所有 F6/F7 释放事件
                return 1;
            }
        }
    }
    // 对于其他按键或未处理的事件，传递给链中的下一个钩子
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

// 启用/禁用功能
void ToggleActiveState(HWND hWnd) {
    is_active = !is_active;
    UpdateTrayMenu(hWnd);
}

// --- 托盘图标和菜单函数 ---

// 修正：使用传入的 hWnd 作为图标宿主
void AddTrayIcon(HWND hWnd) { 
    
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    
    // 使用稳定的主窗口句柄
    nid.hWnd = hWnd; 
    
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    nid.uCallbackMessage = WM_TRAY_ICON_MESSAGE;   
    
    // 设置图标
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    // 修正：使用 wcsncpy 替换有兼容性问题的 wcscpy_s
    wcsncpy(nid.szTip, L"C++ Key Mapper - F6/F7", 128);
    nid.szTip[127] = L'\0'; // 确保 NUL 终止

    // 添加图标到任务栏
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND /*hWnd*/) {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void UpdateTrayMenu(HWND hWnd) {
    HMENU hMenu = CreatePopupMenu();
    
    // 创建菜单项
    UINT stateFlags = MF_STRING;
    if (is_active) {
        stateFlags |= MF_CHECKED; 
    }
    // 修正：明确调用 InsertMenuW (Unicode)
    InsertMenuW(hMenu, 0, MF_BYPOSITION | stateFlags, ID_ENABLE_DISABLE, L"Enable/Disable Mapping");
    
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    
    // 修正：明确调用 InsertMenuW (Unicode)
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_QUIT, L"Exit");
    
    POINT pt;
    GetCursorPos(&pt);
    
    SetForegroundWindow(hWnd); 
    
    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
    
    DestroyMenu(hMenu);
}

// --- 窗口过程函数 ---
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // 设置全局低级键盘钩子
            hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

            if (hHook == NULL) {
                // 修正：明确调用 MessageBoxW (Unicode)
                MessageBoxW(NULL, L"Failed to install keyboard hook!", L"Error", MB_ICONERROR);
                PostQuitMessage(0); 
            }
            // 传递稳定的窗口句柄
            AddTrayIcon(hWnd); 
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_ENABLE_DISABLE:
                    ToggleActiveState(hWnd);
                    break;
                case ID_QUIT:
                    PostQuitMessage(0);
                    break;
            }
            break;
        }

        case WM_TRAY_ICON_MESSAGE: {
            switch (LOWORD(lParam)) {
                case WM_RBUTTONUP: 
                    UpdateTrayMenu(hWnd);
                    break;
            }
            break;
        }

        case WM_DESTROY: {
            if (hHook) UnhookWindowsHookEx(hHook);
            RemoveTrayIcon(hWnd);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// --- WinMain 主函数 (Win32 Entry Point) ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
    const wchar_t CLASS_NAME[] = L"HiddenKeyMapperClass";
    
    // 1. 注册窗口类
    WNDCLASSW wc = {}; // WNDCLASSW 是 WNDCLASS 在 Unicode 模式下的显式名称
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME; 

    // 修正：明确调用 RegisterClassW (Unicode)
    RegisterClassW(&wc); 

    // 2. 创建隐藏的主窗口
    // 修正：明确调用 CreateWindowExW (Unicode)
    HWND hWnd = CreateWindowExW(
        0,                               
        CLASS_NAME,                      
        L"C++ Key Mapper",               
        0,                               // 样式 0 = 完全隐藏
        CW_USEDEFAULT, CW_USEDEFAULT,     
        CW_USEDEFAULT, CW_USEDEFAULT,     
        NULL,                            
        NULL,                            
        hInstance,                       
        NULL                             
    );

    if (hWnd == NULL) {
        return 0;
    }
    
    // 3. 消息循环
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


// --- 解决链接错误：MinGW x64 入口点修正 ---
int main() {
    // 使用 GetCommandLineA() 来匹配 WinMain 的 LPSTR 参数。
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);

}

