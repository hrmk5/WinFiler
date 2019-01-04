#include <Windows.h>
#include <CommCtrl.h>
#include "UI.h"

HFONT guiFont;

bool CALLBACK SetFont(HWND child, LPARAM font) {
	SendMessage(child, WM_SETFONT, font, true);
	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static UI ui;

	switch (msg) {
	case WM_CREATE:
	{
		InitCommonControls();

		ui.initialize(hWnd);
		
		NONCLIENTMETRICS metrics;
		metrics.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
		guiFont = CreateFontIndirect(&metrics.lfMessageFont);
		EnumChildWindows(hWnd, reinterpret_cast<WNDENUMPROC>(SetFont), reinterpret_cast<LPARAM>(guiFont));
		return 0;
	}
	case WM_DESTROY:
		DeleteObject(guiFont);
		PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		ui.onCommand(LOWORD(wParam), HIWORD(wParam), lParam);
		return 0;
	case WM_SIZE:
		ui.onResize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WinFiler";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	HWND window = CreateWindow(
		wcex.lpszClassName, L"WinFiler", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 680, 480,
		NULL, NULL, hInstance, NULL);

	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}