#pragma once
#include <Windows.h>

void gotoxy(int x, int y) {

    COORD pos = { x,y }; //x, y ��ǥ ����
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); //Ŀ�� ����
}
