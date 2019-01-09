#pragma once

#include <array>
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <Shlwapi.h>
#include <fmt/format.h>
#include "Entry.h"
#include "EntryListView.h"

class UI {
public:
	UI();
	~UI();

	void initialize(HWND hWnd);
	void onCommand(WORD id, WORD code, LPARAM lParam);
	void onNotify(LPARAM lParam);
	void onResize(int width, int height);

	void changeDirectory(const std::wstring& directory);
private:
	int windowWidth, windowHeight;
	HWND pathEdit;
	std::unique_ptr<ListViewEx> entryListView;

	std::wstring currentDirectory;
	std::vector<Entry> entries;
};

