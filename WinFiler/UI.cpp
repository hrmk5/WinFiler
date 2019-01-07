#include "UI.h"

UI::UI() {
}

UI::~UI() {
	ImageList_Destroy(imageList);
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
	entryListView = CreateWindow(
		WC_ENTRYLISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd, reinterpret_cast<HMENU>(ID_ENTRYLISTVIEW), NULL, NULL);
	EntryListView_SetVectorPtr(entryListView, &entries);

	onResize(windowWidth, windowHeight);

	// Set entry list columns
	/*std::wstring name[] = { L"ファイル名", L"最終更新日時", L"サイズ" };
	int width[] = { 200, 200, 200 };

	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	for (int i = 0; i < sizeof(name) / sizeof(name[0]); i++) {
		col.cx = width[i];
		col.pszText = &*name[i].begin();
		col.iSubItem = i;
		ListView_InsertColumn(entryList, i, &col);
	}*/

	changeDirectory(L"C:\\");
}

void UI::onCommand(WORD id, WORD code, LPARAM lParam) {
}

void UI::onNotify(LPARAM lParam) {
}

void UI::onResize(int width, int height) {
	MoveWindow(pathEdit, 5, 5, width - 10, 25, FALSE);
	MoveWindow(entryListView, 5, 35, width - 10, height - 40, TRUE);
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
			//entry.timeStr = L"";
			//entry.sizeStr = L"";
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
