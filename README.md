# KeyMapper
###能进行空闲键盘管理的小工具
支持微软拼音输入法
```cpp
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
'''

        Sleep(5);
    }
}
