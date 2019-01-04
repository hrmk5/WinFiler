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
		WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT,
		0, 0, 0, 0,
		hWnd, reinterpret_cast<HMENU>(ID_ENTRYLIST), NULL, NULL);
	onResize(windowWidth, windowHeight);

	// Set entry list columns
	std::wstring name[] = { L"ƒtƒ@ƒCƒ‹–¼" };
	int width[] = { 200 };

	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	for (int i = 0; i < sizeof(name) / sizeof(name[0]); i++) {
		col.cx = width[i];
		col.pszText = &*name[i].begin();
		col.iSubItem = i;
		ListView_InsertColumn(entryList, i, &col);
	}
}

void UI::onCommand(WORD id, WORD code, LPARAM lParam) {
}

void UI::onResize(int width, int height) {
	MoveWindow(pathEdit, 5, 5, width - 10, 25, FALSE);
	MoveWindow(entryList, 5, 35, width - 10, height - 40, TRUE);
}
