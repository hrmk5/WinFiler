#pragma once

#include <array>
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <Shlwapi.h>
#include <fmt/format.h>

enum class EntryType {
	DIRECTORY,
	FILE,
};

struct Entry {
	EntryType type;
	std::wstring name;
	std::wstring path;
	long size;
	SYSTEMTIME time;
};

class UI {
public:
	UI();
	~UI();

	void initialize(HWND hWnd);
	void onCommand(WORD id, WORD code, LPARAM lParam);
	void onResize(int width, int height);

	void changeDirectory(const std::wstring& directory);
private:
	int windowWidth, windowHeight;
	HWND pathEdit, entryList;

	std::wstring currentDirectory;
	std::vector<Entry> entries;
};

