#include "EntryListView.h"

constexpr int ROW_SPACE = 3;

ListViewEx::ListViewEx(HWND hWnd, HMENU id) {
	this->hWnd = CreateWindow(
		WC_ENTRYLISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd, id, NULL, this);

	entries = nullptr;
	height = 0;
	hoverBrush = CreateSolidBrush(RGB(220, 220, 220));
}

void ListViewEx::Register() {
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ListViewEx::WndProc;
	wc.lpszClassName = WC_ENTRYLISTVIEW;
	wc.cbWndExtra = sizeof(ListViewEx*);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);
}

void ListViewEx::SetVectorPtr(std::vector<Entry>* vec) {
	entries = vec;
}

void ListViewEx::Move(int x, int y, int width, int height, bool repaint) {
	MoveWindow(hWnd, x, y, width, height, repaint);
}

LRESULT CALLBACK ListViewEx::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	auto lv = reinterpret_cast<ListViewEx*>(GetWindowLongPtr(hWnd, 0));
	switch (msg) {
	case WM_CREATE:
	{
		LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		lv = reinterpret_cast<ListViewEx*>(pcs->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(lv));

		// SCROLLINFO ��������
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPos = 0;
		si.nMin = 0;
		SetScrollInfo(hWnd, SB_VERT, &si, FALSE);
		return 0;
	}
	case WM_DESTROY:
		return 0;
	case WM_VSCROLL:
	{
		// ���݂� SCROLLINFO ���擾
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		GetScrollInfo(hWnd, SB_VERT, &si);

		// �X�N���[����
		int dy = 0;

		switch (LOWORD(wParam)) {
		case SB_LINEUP:
			dy = -lv->rowHeight;
			break;
		case SB_LINEDOWN:
			dy = lv->rowHeight;
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
		// �ĕ`��
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
		dy = delta / 120 * -(lv->rowHeight * 2);

		SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
		// �ĕ`��
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_MOUSEMOVE:
		// �ĕ`��
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_PAINT:
		lv->OnPaint();
		return 0;
	case WM_SETFONT:
	{
		lv->font = reinterpret_cast<HFONT>(wParam);
		// �t�H���g�̍������擾
		auto hdc = GetDC(hWnd);
		SelectObject(hdc, lv->font);
		TEXTMETRIC metrics;
		GetTextMetrics(hdc, &metrics);
		// �s�̍������v�Z
		lv->rowHeight = metrics.tmHeight + ROW_SPACE;
		return 0;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void ListViewEx::OnPaint() {
	// ��ʂ̃T�C�Y���擾
	RECT rect;
	GetClientRect(hWnd, &rect);

	// ���݂� SCROLLINFO ���擾
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &si);
	
	// �J�[�\���ʒu���擾
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(hWnd, &cursorPos);

	PAINTSTRUCT ps;
	HDC _hdc = BeginPaint(hWnd, &ps);
	HDC hdc = CreateCompatibleDC(_hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(_hdc, rect.right, rect.bottom);
	SelectObject(hdc, bitmap);

	FillRect(hdc, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

	// �t�H���g��ݒ�
	SelectObject(hdc, font);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	int count = 0;
	int maxY = 0;
	if (entries != nullptr) {
		for (const auto& entry : *entries) {
			auto y = rowHeight * count - si.nPos;
			if (cursorPos.y >= y && cursorPos.y < y + rowHeight) {
				// �J�[�\�����d�Ȃ��Ă�����w�i��`��
				RECT background = { 0, y, rect.right, y + rowHeight };
				FillRect(hdc, &background, hoverBrush);
			}

			// �t�@�C������`��
			RECT textRect = { 0, y, rect.right, y + rowHeight };
			DrawText(hdc, entry.name.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER);
			count++;
		}
		maxY = rowHeight * count;
	}

	BitBlt(_hdc, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

	DeleteObject(bitmap);
	DeleteDC(hdc);

	EndPaint(hWnd, &ps);

	// �X�N���[���o�[
	si.nPage = rect.bottom;
	si.nMax = maxY;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
}
