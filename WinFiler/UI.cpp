#include "UI.h"

UI::UI() {
}

UI::~UI() {
}

constexpr int ID_PATHEDIT = 1;
constexpr int ID_ENTRYLISTVIEW = 2;

void UI::initialize(HWND hWnd) {
	RECT rect;
	GetClientRect(hWnd, &rect);
	int windowWidth = rect.right;
	int windowHeight = rect.bottom;

	pathEdit = CreateWindow(
		L"EDIT", L"C:\\", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		0, 0, 0, 0,
		hWnd, reinterpret_cast<HMENU>(ID_PATHEDIT), NULL, NULL);
	entryListView = std::make_unique<ListViewEx>(hWnd, reinterpret_cast<HMENU>(ID_ENTRYLISTVIEW));
	entryListView->SetVectorPtr(&entries);

	onResize(windowWidth, windowHeight);

	changeDirectory(L"C:\\");
}

void UI::onCommand(WORD id, WORD code, LPARAM lParam) {
}

void UI::onNotify(LPARAM lParam) {
}

void UI::onResize(int width, int height) {
	MoveWindow(pathEdit, 5, 5, width - 10, 25, FALSE);
	entryListView->Move(5, 35, width - 10, height - 40, true);
}

void UI::changeDirectory(const std::wstring& directory) {
	currentDirectory = directory;
	entries.clear();

	int count = 0;

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile((L"\\\\?\\" + directory + L"\\*").c_str(), &findData);
	do {
		Entry entry;
		entry.name = findData.cFileName;

		// Join the directory and cFileName
		wchar_t path[256];
		PathCombine(path, directory.c_str(), entry.name.c_str());
		entry.path = path;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			entry.type = EntryType::DIRECTORY;
		} else {
			entry.type = EntryType::FILE;
			// Size
			entry.size = MAKELONG(findData.nFileSizeLow, findData.nFileSizeHigh);
			//entry.sizeStr = fmt::format(L"{} B", entry.size);
			// Last modified time
			FileTimeToSystemTime(&findData.ftLastWriteTime, &entry.time);
			//entry.timeStr = fmt::format(L"{}/{:0>2}/{:0>2} {:0>2}:{:0>2}",
			//		entry.time.wYear, entry.time.wMonth, entry.time.wDay, entry.time.wHour, entry.time.wMinute, entry.time.wSecond);
		}

		// Get file icon
		HICON icon;
		if (entry.type == EntryType::DIRECTORY) {
			SHSTOCKICONINFO sii;
			sii.cbSize = sizeof(sii);
			SHGetStockIconInfo(SIID_FOLDER, SHGSI_ICON | SHGSI_SMALLICON, &sii);
			icon = sii.hIcon;
		} else if (entry.type == EntryType::FILE) {
			SHFILEINFO sfi;
			SHGetFileInfo(entry.path.c_str(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
			icon = sfi.hIcon;
		}

		entries.push_back(entry);
		count++;
	} while (FindNextFile(hFind, &findData));
}
