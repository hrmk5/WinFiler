#pragma once

#include <vector>
#include <map>
#include <functional>
#include <any>
#include <tchar.h>
#include <Windows.h>
#include <windowsx.h>
#include <fmt/format.h>

#define WC_ENTRYLISTVIEW _T("EntryListView")

struct ListViewExColumn {
	int width;
	std::wstring header;
	std::function<std::wstring(const std::any& v)> get;
	std::function<HICON(const std::any& v)> getIcon = [](const std::any& v) { return nullptr; };
};

enum class ButtonType {
	LEFT,
	RIGHT,
	MIDDLE,
	X1,
	X2,
};

enum class ListViewExClickedLocation {
	HEADER,
	ITEM,
	SPLITTER,
};

struct ListViewExOptions {
	// çÄñ⁄Ç™ëIëÇ≥ÇÍÇƒÇ¢Ç»Ç¢èÍçáÇÕ index Ç™ -1ÅAitem Ç™ nullptr Ç…Ç»ÇÈ
	std::function<void(ButtonType type, ListViewExClickedLocation location)> OnDoubleClicked = [](ButtonType t, ListViewExClickedLocation l) {};
	std::function<void(ButtonType type, ListViewExClickedLocation location)> OnMouseDown = [](ButtonType t, ListViewExClickedLocation l) {};;
	std::function<void(ButtonType type, ListViewExClickedLocation location)> OnMouseUp = [](ButtonType t, ListViewExClickedLocation l) {};;
};

class ListViewEx {
public:
	explicit ListViewEx(HWND hWnd, HMENU id, ListViewExOptions* options);

	static void Register();

	void AddColumn(const ListViewExColumn& column);
	void AddItem(const std::any& value);
	void DeleteAllItems();
	std::vector<ListViewExColumn> GetAllColumns();
	std::vector<int> GetSelectedIndexes();
	std::vector<std::any> GetSelectedItems();
	void Move(int x, int y, int width, int height, bool repaint);
private:
	HWND hWnd;

	HBRUSH hoverBrush;
	HBRUSH colorBrush;
	HBRUSH backgroundBrush;

	ListViewExOptions options;
	std::vector<std::any> items;
	std::vector<ListViewExColumn> columns;
	HFONT font;
	HCURSOR currentCursor;
	int leftPadding;
	int columnHeaderHeight;
	int rowHeight;
	int rowWidth;
	std::map<int, bool> selectedIndexes;
	int lastSelectedIndex;
	const ListViewExColumn* hoveringSplitterColumn;
	bool splitterIsMoving = false;

	ListViewExClickedLocation GetClickedLocation();

	static LRESULT CALLBACK WndProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos);
	void OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos);
	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys);
	void OnMouseHWheel(HWND hWnd, int delta);
	void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);
	void OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags);
	void OnSetFont(HWND hWndCtl, HFONT hfont, BOOL fRedraw);
	BOOL OnSetCursor(HWND hWnd, HWND hWndCursor, UINT codeHitTest, UINT msg);
	void OnPaint(HWND hWnd);
};
