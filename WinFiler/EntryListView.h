#pragma once

#include <vector>
#include <tchar.h>
#include <Windows.h>
#include <fmt/format.h>
#include "Entry.h"

#define WC_ENTRYLISTVIEW _T("EntryListView")

struct EntryListView_Data {
	std::vector<Entry>* entries;
	HFONT font;
	int height;
	int rowHeight;
};

static void OnPaint(HWND hWnd, EntryListView_Data& data);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void EntryListView_SetVectorPtr(HWND hWnd, std::vector<Entry>* vec);
void EntryListView_Register();