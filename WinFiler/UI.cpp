#include "UI.h"

UI::UI() {
}

UI::~UI() {
}

constexpr int ID_PATHEDIT = 1;
constexpr int ID_ENTRYLIST = 2;

void UI::initialize(HWND hWnd) {
	RECT rect;
	GetClientRect(hWnd, &rect);
	int windowWidth = rect.right;
	int windowHeight = rect.bottom;

	pathEdit = CreateWindow(
		L"EDIT", L"C:\\", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		0, 0, 0, 0,
		hWnd, reinterpret_cast<HMENU>(ID_PATHEDIT), NULL, NULL);
	entryList = CreateWindow(
		WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE ,
		0, 0, 0, 0,
		hWnd, reinterpret_cast<HMENU>(ID_ENTRYLIST), NULL, NULL);
	SetWindowTheme(entryList, L"Explorer", NULL);

	onResize(windowWidth, windowHeight);

	// Set entry list columns
	std::wstring name[] = { L"ファイル名", L"最終更新日時", L"サイズ" };
	int width[] = { 200, 200, 200 };

	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	for (int i = 0; i < sizeof(name) / sizeof(name[0]); i++) {
		col.cx = width[i];
		col.pszText = &*name[i].begin();
		col.iSubItem = i;
		ListView_InsertColumn(entryList, i, &col);
	}

	changeDirectory(L"C:\\Users\\Shinsuke\\Documents");
}

void UI::onCommand(WORD id, WORD code, LPARAM lParam) {
}

void UI::onResize(int width, int height) {
	MoveWindow(pathEdit, 5, 5, width - 10, 25, FALSE);
	MoveWindow(entryList, 5, 35, width - 10, height - 40, TRUE);
}

void UI::changeDirectory(const std::wstring& directory) {
	currentDirectory = directory;
	entries.clear();
	ListView_DeleteAllItems(entryList);

	LVITEM item;
	item.mask = LVIF_TEXT;
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
			entry.size = MAKELONG(findData.nFileSizeLow, findData.nFileSizeHigh);
			FileTimeToSystemTime(&findData.ftLastWriteTime, &entry.time);
		}
		entries.push_back(entry);

		item.iItem = count;

		// File name
		item.iSubItem = 0;
		item.pszText = &*entry.name.begin();
		ListView_InsertItem(entryList, &item);

		if (entry.type == EntryType::FILE) {
			// Last modified time
			item.iSubItem = 1;
			std::wstring str = fmt::format(L"{}/{:0>2}/{:0>2} {:0>2}:{:0>2}",
				entry.time.wYear, entry.time.wMonth, entry.time.wDay, entry.time.wHour, entry.time.wMinute, entry.time.wSecond);
			item.pszText = &*str.begin();
			ListView_SetItem(entryList, &item);

			// Size
			item.iSubItem = 2;
			str = fmt::format(L"{} B", entry.size);
			item.pszText = &*str.begin();
			ListView_SetItem(entryList, &item);
		}

		count++;
	} while (FindNextFile(hFind, &findData));
}
