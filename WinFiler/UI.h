#pragma once

#include <array>
#include <iostream>
#include <Windows.h>
#include <CommCtrl.h>

class UI {
public:
	UI();
	~UI();

	void initialize(HWND hWnd);
	void onCommand(WORD id, WORD code, LPARAM lParam);
	void onResize(int width, int height);
private:
	int windowWidth, windowHeight;
	HWND pathEdit, entryList;
};

