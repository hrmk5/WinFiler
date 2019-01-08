#include "EntryListView.h"

constexpr int ROW_SPACE = 3;

static void OnPaint(HWND hWnd, EntryListView_Data& data) {
	// 画面のサイズを取得
	RECT rect;
	GetClientRect(hWnd, &rect);

	// 現在の SCROLLINFO を取得
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &si);
	
	// カーソル位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(hWnd, &cursorPos);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	FillRect(hdc, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

	// フォントを設定
	SelectObject(hdc, data.font);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	int count = 0;
	int maxY = 0;
	if (data.entries != nullptr) {
		for (const auto& entry : *data.entries) {
			auto y = data.rowHeight * count - si.nPos;
			if (cursorPos.y >= y && cursorPos.y < y + data.rowHeight) {
				// カーソルが重なっていたら背景を描画
				RECT background = { 0, y, rect.right, y + data.rowHeight };
				FillRect(hdc, &background, data.hoverBrush);
			}

			// ファイル名を描画
			RECT textRect = { 0, y, rect.right, y + data.rowHeight };
			DrawText(hdc, entry.name.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER);
			count++;
		}
		maxY = data.rowHeight * count;
	}

	EndPaint(hWnd, &ps);

	// スクロールバー
	si.nPage = rect.bottom;
	si.nMax = maxY;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	auto data = reinterpret_cast<EntryListView_Data*>(GetWindowLongPtr(hWnd, 0));
	switch (msg) {
	case WM_NCCREATE:
		// EntryListView_Data を初期化
		data = new EntryListView_Data;
		data->entries = nullptr;
		data->height = 0;
		data->hoverBrush = CreateSolidBrush(RGB(220, 220, 220));
		SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(data));
		return 1;
	case WM_NCDESTROY:
		if (data != nullptr) {
			DeleteObject(data->hoverBrush);
			delete data;
		}
		return 0;
	case WM_CREATE:
		// SCROLLINFO を初期化
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPos = 0;
		si.nMin = 0;
		SetScrollInfo(hWnd, SB_VERT, &si, FALSE);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_VSCROLL:
	{
		// 現在の SCROLLINFO を取得
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		GetScrollInfo(hWnd, SB_VERT, &si);

		// スクロール量
		int dy = 0;

		switch (LOWORD(wParam)) {
		case SB_LINEUP:
			dy = -data->rowHeight;
			break;
		case SB_LINEDOWN:
			dy = data->rowHeight;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			dy = HIWORD(wParam) - si.nPos;
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
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &si);

		auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
		int dy = 0;
		dy = delta / 120 * -(data->rowHeight * 2);

		SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
		// 再描画
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_MOUSEMOVE:
		// 再描画
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_PAINT:
		OnPaint(hWnd, *data);
		return 0;
	case WM_SETFONT:
	{
		data->font = reinterpret_cast<HFONT>(wParam);
		// フォントの高さを取得
		auto hdc = GetDC(hWnd);
		SelectObject(hdc, data->font);
		TEXTMETRIC metrics;
		GetTextMetrics(hdc, &metrics);
		// 行の高さを計算
		data->rowHeight = metrics.tmHeight + ROW_SPACE;
		return 0;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void EntryListView_SetVectorPtr(HWND hWnd, std::vector<Entry>* vec) {
	auto data = reinterpret_cast<EntryListView_Data*>(GetWindowLongPtr(hWnd, 0));
	data->entries = vec;
}

void EntryListView_Register() {
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WC_ENTRYLISTVIEW;
	wc.cbWndExtra = sizeof(EntryListView_Data*);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);
}
