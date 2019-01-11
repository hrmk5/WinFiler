#pragma once

#include <string>
#include <Windows.h>

enum class EntryType {
	DIRECTORY,
	FILE,
};

struct Entry {
	EntryType type;
	HICON icon;
	std::wstring name;
	std::wstring path;
	long size;
	SYSTEMTIME time;
};
