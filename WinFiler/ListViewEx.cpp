#include "ListViewEx.h"

constexpr int ROW_SPACE = 4;
constexpr int ICON_SIZE = 16;
constexpr int MIN_COLUMN_WIDTH = 15;

ListViewEx::ListViewEx(HWND hWnd, HMENU id, ListViewExOptions* options) : rowWidth(0), leftPadding(7) {
	this->hWnd = CreateWindow(
		WC_ENTRYLISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd, id, NULL, this);

	hoverBrush = CreateSolidBrush(RGB(220, 220, 220));
	colorBrush = CreateSolidBrush(RGB(126, 229, 162));
	backgroundBrush = CreateSolidBrush(RGB(255, 255, 255));
	currentCursor = LoadCursor(NULL, IDC_ARROW);
	if (options != nullptr) {
		this->options = *options;
	}
}

void ListViewEx::Register() {
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ListViewEx::WndProc_;
	wc.lpszClassName = WC_ENTRYLISTVIEW;
	wc.cbWndExtra = sizeof(ListViewEx*);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);
}

void ListViewEx::AddColumn(const ListViewExColumn& column) {
	columns.push_back(column);
	rowWidth += column.width;
}

void ListViewEx::AddItem(const std::any& value) {
	items.push_back(value);
}

void ListViewEx::DeleteAllItems() {
	items.clear();
}

std::vector<ListViewExColumn> ListViewEx::GetAllColumns() {
	return columns;
}

std::vector<int> ListViewEx::GetSelectedIndexes() {
	std::vector<int> indexes;
	for (const auto& [index, selected] : selectedIndexes) {
		if (selected) {
			indexes.push_back(index);
		}
	}

	return indexes;
}

std::vector<std::any> ListViewEx::GetSelectedItems() {
	std::vector<std::any> items;
	for (const auto& [index, selected] : selectedIndexes) {
		if (selected) {
			items.push_back(items[index]);
		}
	}

	return items;
}

void ListViewEx::Move(int x, int y, int width, int height, bool repaint) {
	MoveWindow(hWnd, x, y, width, height, repaint);
}

LRESULT CALLBACK ListViewEx::WndProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		auto lv = reinterpret_cast<ListViewEx*>(pcs->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(lv));
		return TRUE;
	}

	auto lv = reinterpret_cast<ListViewEx*>(GetWindowLongPtr(hWnd, 0));
	return lv->WndProc(hWnd, msg, wParam, lParam);
}

ListViewExClickedLocation ListViewEx::GetClickedLocation() {
	if (hoveringSplitterColumn != nullptr) {
		return ListViewExClickedLocation::SPLITTER;
	}

	// カーソル位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(hWnd, &cursorPos);

	return cursorPos.y <= columnHeaderHeight ? ListViewExClickedLocation::HEADER : ListViewExClickedLocation::ITEM;
}

LRESULT CALLBACK ListViewEx::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hwnd, WM_VSCROLL, OnVScroll);
	HANDLE_MSG(hwnd, WM_HSCROLL, OnHScroll);
	HANDLE_MSG(hwnd, WM_MOUSEWHEEL, OnMouseWheel);
	HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouseMove);
	HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
	HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLButtonUp);
	HANDLE_MSG(hwnd, WM_SETFONT, OnSetFont);
	HANDLE_MSG(hwnd, WM_SETCURSOR, OnSetCursor);
	HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	case WM_RBUTTONDOWN:
		options.OnMouseDown(ButtonType::RIGHT, GetClickedLocation());
		return 0;
	case WM_RBUTTONUP:
		options.OnMouseUp(ButtonType::RIGHT, GetClickedLocation());
		return 0;
	case WM_RBUTTONDBLCLK:
		options.OnDoubleClicked(ButtonType::RIGHT, GetClickedLocation());
		return 0;
	case WM_MBUTTONDOWN:
		options.OnMouseDown(ButtonType::MIDDLE, GetClickedLocation());
		return 0;
	case WM_MBUTTONUP:
		options.OnMouseUp(ButtonType::MIDDLE, GetClickedLocation());
		return 0;
	case WM_MBUTTONDBLCLK:
		options.OnDoubleClicked(ButtonType::MIDDLE, GetClickedLocation());
		return 0;
	case WM_XBUTTONDOWN:
		options.OnMouseDown(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ButtonType::X1 : ButtonType::X2, GetClickedLocation());
		return 0;
	case WM_XBUTTONUP:
		options.OnMouseUp(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ButtonType::X1 : ButtonType::X2, GetClickedLocation());
		return 0;
	case WM_XBUTTONDBLCLK:
		options.OnDoubleClicked(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ButtonType::X1 : ButtonType::X2, GetClickedLocation());
		return 0;
	case WM_MOUSEHWHEEL:
		OnMouseHWheel(hwnd, HIWORD(wParam));
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

BOOL ListViewEx::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) {
	// SCROLLINFO を初期化
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	si.nPos = 0;
	si.nMin = 0;
	SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

	SCROLLINFO hsi;
	hsi.cbSize = sizeof(hsi);
	hsi.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	hsi.nPos = 0;
	hsi.nMin = 0;
	SetScrollInfo(hWnd, SB_HORZ, &hsi, FALSE);

	return TRUE;
}

void ListViewEx::OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos) {
	// 現在の SCROLLINFO を取得
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &si);

	// スクロール量
	int dy = 0;

	switch (code) {
	case SB_LINEUP:
		dy = -rowHeight;
		break;
	case SB_LINEDOWN:
		dy = rowHeight;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		dy = pos - si.nPos;
		break;
	case SB_PAGEDOWN:
		dy = -static_cast<int>(si.nPage);
		break;
	case SB_PAGEUP:
		dy = si.nPage;
		break;
	default:
		dy = 0;
		break;
	}

	SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos) {
	// 現在の SCROLLINFO を取得
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &si);

	// スクロール量
	int dx = 0;

	switch (code) {
	case SB_LINELEFT:
		dx = -10;
		break;
	case SB_LINERIGHT:
		dx = 10;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		dx = pos - si.nPos;
		break;
	case SB_PAGELEFT:
		dx = -static_cast<int>(si.nPage);
		break;
	case SB_PAGERIGHT:
		dx = si.nPage;
		break;
	default:
		dx = 0;
		break;
	}

	SetScrollPos(hWnd, SB_HORZ, si.nPos + dx, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys) {
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_VERT, &si);

	int dy = 0;
	dy = zDelta / 120 * -(rowHeight * 2);

	SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnMouseHWheel(HWND hWnd, int delta) {
	// TODO: マウスを左に回転させると delta が 65146 になる
	/*SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_HORZ, &si);

	int dx = 0;
	dx = delta / 120 * 20;

	SetScrollPos(hWnd, SB_HORZ, si.nPos + dx, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);*/
}

void ListViewEx::OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags) {
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnLButtonDown(HWND hWnd, BOOL doubleClick, int x, int y, UINT keyFlags) {
	int index = -1;

	if (!doubleClick) {
		if (x < rowWidth) {
			// 垂直スクロールバーを取得
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);

			if (y < columnHeaderHeight) {
				if (hoveringSplitterColumn != nullptr) {
					// 分割バーがクリックされていた場合は現在の列の幅を保存する
					splitterIsMoving = true;
				}
				// TODO: 見出しにクリックした場合はソート
			} else {
				index = (y + si.nPos - columnHeaderHeight) / rowHeight;

				if (keyFlags & MK_CONTROL) {
					// コントロールキーを押していて、既に選択されていたら選択を解除する
					if (selectedIndexes[index]) {
						selectedIndexes[index] = false;
					} else {
						selectedIndexes[index] = true;
						lastSelectedIndex = index;
					}
				} else if (keyFlags & MK_SHIFT) {
					// シフトキーを押していたら最後に選択した項目までのすべての項目を選択
					if (selectedIndexes.empty()) {
						selectedIndexes[index] = true;
						lastSelectedIndex = index;
					} else {
						selectedIndexes.clear();
						selectedIndexes[lastSelectedIndex] = true;
						for (int i = 0; i < abs(index - lastSelectedIndex); i++) {
							selectedIndexes[index > lastSelectedIndex ? lastSelectedIndex + 1 + i : index + i] = true;
						}
					}
				} else {
					// コントロールキーが押されていなかったら現在の選択範囲を消去
					selectedIndexes.clear();
					selectedIndexes[index] = true;
					lastSelectedIndex = index;
				}
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}

		options.OnDoubleClicked(ButtonType::LEFT, GetClickedLocation());
	} else {
		options.OnMouseDown(ButtonType::LEFT, GetClickedLocation());
	}
}

void ListViewEx::OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags) {
	hoveringSplitterColumn = nullptr;
	splitterIsMoving = false;
	options.OnMouseUp(ButtonType::LEFT, GetClickedLocation());
}

void ListViewEx::OnSetFont(HWND hWndCtl, HFONT hFont, BOOL fRedraw) {
	font = hFont;
	// フォントの高さを取得
	auto hdc = GetDC(hWnd);
	SelectObject(hdc, font);
	TEXTMETRIC metrics;
	GetTextMetrics(hdc, &metrics);
	// 行の高さを計算
	rowHeight = metrics.tmHeight + ROW_SPACE;
	columnHeaderHeight = metrics.tmHeight + 10;
}

BOOL ListViewEx::OnSetCursor(HWND hWnd, HWND hWndCursor, UINT codeHitTest, UINT msg) {
	SetCursor(currentCursor);
	return FALSE;
}

void ListViewEx::OnPaint(HWND hWnd) {
	// 画面のサイズを取得
	RECT rect;
	GetClientRect(hWnd, &rect);

	// 垂直スクロールバー
	SCROLLINFO verticalScrollInfo;
	verticalScrollInfo.cbSize = sizeof(verticalScrollInfo);
	verticalScrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &verticalScrollInfo);

	// 平行スクロールバー
	SCROLLINFO horizontalScrollInfo;
	horizontalScrollInfo.cbSize = sizeof(horizontalScrollInfo);
	horizontalScrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &horizontalScrollInfo);
	
	// カーソル位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(hWnd, &cursorPos);

	PAINTSTRUCT ps;
	HDC _hdc = BeginPaint(hWnd, &ps);
	HDC hdc = CreateCompatibleDC(_hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(_hdc, rect.right, rect.bottom);
	SelectObject(hdc, bitmap);

	FillRect(hdc, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

	// フォントを設定
	SelectObject(hdc, font);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));

	int count = 0;
	for (const auto& item : items) {
		// 描画する Y 座標
		auto y = columnHeaderHeight + rowHeight * count - verticalScrollInfo.nPos;

		// 選択されていたら背景を描画
		RECT background = { 0, y, rowWidth, y + rowHeight };
		if (selectedIndexes[count]) {
			FillRect(hdc, &background, colorBrush);
			background.left += 1;
			background.top += 1;
			background.right -= 1;
			background.bottom -= 1;
		}
		// カーソルが重なっていたら背景を描画
		if (cursorPos.y >= y && cursorPos.y < y + rowHeight && cursorPos.x < rowWidth) {
			FillRect(hdc, &background, hoverBrush);
		}

		// 描画する X 座標
		int x = 0;
		for (const auto& column : columns) {
			int textX = x;

			// アイコンを描画
			auto icon = column.getIcon(item);
			if (icon != nullptr) {
				ICONINFO iconInfo;
				GetIconInfo(icon, &iconInfo);
				DrawIconEx(hdc, x + leftPadding - horizontalScrollInfo.nPos, y + ((rowHeight - ICON_SIZE) / 2), icon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
				textX += ICON_SIZE + 3 + leftPadding;
			}

			// 文字列を描画
			RECT textRect = { textX - horizontalScrollInfo.nPos, y, textX - horizontalScrollInfo.nPos + column.width - (textX - x), y + rowHeight };
			DrawText(hdc, column.get(item).c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER);

			x += column.width + leftPadding;
		}

		count++;
	}

	// 列の見出しを描画
	SetTextColor(hdc, RGB(100, 100, 100));
	int headerX = 0;
	bool splitterHover = false;
	for (auto& column : columns) {
		if (hoveringSplitterColumn == &column && splitterIsMoving) {
			column.width = cursorPos.x - headerX;
			if (column.width <= MIN_COLUMN_WIDTH) {
				column.width = MIN_COLUMN_WIDTH;
			}
		}
		int right = headerX + column.width - horizontalScrollInfo.nPos;

		// 背景
		RECT backgroundRect{ headerX + 1 - horizontalScrollInfo.nPos, 0, right, columnHeaderHeight };
		FillRect(hdc, &backgroundRect, backgroundBrush);

		// 見出し文字列
		RECT headerRect{ headerX + leftPadding - horizontalScrollInfo.nPos, 0, right, columnHeaderHeight };
		DrawText(hdc, column.header.c_str(), -1, &headerRect, DT_SINGLELINE | DT_VCENTER);

		// 分割バー
		RECT splitterRect{ right, 0, right + 1, columnHeaderHeight };
		if (cursorPos.x > splitterRect.left - 5 && cursorPos.x < splitterRect.right + 5 &&
			cursorPos.y < splitterRect.bottom && cursorPos.y > 0) {
			// マウスが重なっていたら灰色で描画する
			FillRect(hdc, &splitterRect, hoverBrush);
			splitterHover = true;
			if (hoveringSplitterColumn == nullptr) {
				hoveringSplitterColumn = &column;
			}
		} else {
			FillRect(hdc, &splitterRect, colorBrush);
		}
		headerX += column.width;
	}
	rowWidth = headerX;

	if (splitterHover || splitterIsMoving) {
		// 分割バーにマウスが重なっていたらカーソルを変更
		currentCursor = LoadCursor(NULL, IDC_SIZEWE);
	} else {
		// 重なっていなかったら元に戻す
		currentCursor = LoadCursor(NULL, IDC_ARROW);
		hoveringSplitterColumn = nullptr;
	}

	int maxY = columnHeaderHeight + (rowHeight * count);

	BitBlt(_hdc, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

	DeleteObject(bitmap);
	DeleteDC(hdc);

	EndPaint(hWnd, &ps);

	// 垂直スクロールバー
	verticalScrollInfo.nPage = rect.bottom;
	verticalScrollInfo.nMax = maxY;
	SetScrollInfo(hWnd, SB_VERT, &verticalScrollInfo, TRUE);

	// 平行スクロールバー
	horizontalScrollInfo.nPage = rect.right;
	horizontalScrollInfo.nMax = rowWidth;
	SetScrollInfo(hWnd, SB_HORZ, &horizontalScrollInfo, TRUE);
}
