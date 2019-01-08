#pragma once

#include <vector>
#include <tchar.h>
#include <Windows.h>
#include <fmt/format.h>
#include "Entry.h"

#define WC_ENTRYLISTVIEW _T("EntryListView")

struct EntryListView_Data {
	HBRUSH hoverBrush;
	std::vector<Entry>* entries;
	HFONT font;
	int height;
	int rowHeight;
};

class ListViewEx {
public:
	ListViewEx(HWND hWnd, HMENU id);

	static void Register();

	void SetVectorPtr(std::vector<Entry>* vec);
	void Move(int x, int y, int width, int height, bool repaint);
private:
	HWND hWnd;

	HBRUSH hoverBrush;
	std::vector<Entry>* entries;
	HFONT font;
	int height;
	int rowHeight;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void OnPaint();
};
