#include <Windows.h>
#include <array>

#include "resource.h"

constexpr CHAR X{ 'X' };
constexpr CHAR O{ 'O' };

struct Square {
	CHAR owner{};
	RECT rc{};
};

std::array<Square, 9> g_squares{
	Square{0, {0, 0, 20, 20}}, Square{0, {20, 0, 40, 20}}, Square{0, {40, 0, 60, 20}},
	Square{0, {0, 20, 20, 40}}, Square{0, {20, 20, 40, 40}}, Square{0, {40, 20, 60, 40}},
	Square{0, {0, 40, 20, 60}}, Square{0, {20, 40, 40, 60}}, Square{0, {40, 40, 60, 60}},
};

LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static BOOL s_isXturn{ TRUE };

	switch (uMsg) {

	case WM_SETCURSOR: {
		HCURSOR hCursor{ LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE((s_isXturn == TRUE ? IDC_EX : IDC_CIRCLE))) };
		DestroyCursor(SetCursor(hCursor));
		return FALSE;
	}

	case WM_CREATE: {
		HDC hDC{ GetDC(hWnd) };

		RECT rcClient{};
		GetClientRect(hWnd, &rcClient);

		SetMapMode(hDC, MM_ANISOTROPIC);
		SetWindowExtEx(hDC, 60, 60, NULL);
		SetViewportExtEx(hDC, rcClient.right, rcClient.bottom, NULL);

		// NULL BRUSH
		SelectObject(hDC, GetStockObject(NULL_BRUSH));

		ReleaseDC(hWnd, hDC);
		return 0;
	}

	case WM_RBUTTONDOWN: {
	case WM_LBUTTONDOWN:
		HDC hDC{ GetDC(hWnd) };

		POINT point{ LOWORD(lParam), HIWORD(lParam) };

		DPtoLP(hDC, &point, 1);

		for (SIZE_T i{}; i < g_squares.size(); ++i) {
			if (PtInRect(&g_squares[i].rc, point) && g_squares[i].owner == 0/*NULL*/)
				g_squares[i].owner = (
					(s_isXturn == FALSE ? (s_isXturn = TRUE, O) : (s_isXturn = FALSE, X))
					);
		}

		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);

		// check for a draw or a winner
		BOOL isGameOver{ FALSE };

		// // winner?
		constexpr std::array<std::array<SIZE_T, 3>, 8> WIN_COMBINATIONS{ {
				//rows
				{0, 1, 2},
				{3, 4, 5},
				{6, 7, 8},
				//colms
				{0, 3, 6},
				{1, 4, 7},
				{2, 5, 8},
				//diagonals
				{0, 4, 8},
				{2, 4, 6}
			} };

		for (SIZE_T i{}; i < WIN_COMBINATIONS.size(); ++i) {
			CONST SIZE_T a{ WIN_COMBINATIONS[i][0] };
			CONST SIZE_T b{ WIN_COMBINATIONS[i][1] };
			CONST SIZE_T c{ WIN_COMBINATIONS[i][2] };


			if (g_squares[a].owner == O &&
				g_squares[b].owner == O &&
				g_squares[c].owner == O) {
				MessageBox(hWnd, TEXT("O takes the victory!"), TEXT("Game over"), MB_OK);
				isGameOver = TRUE;
				break;
			}
			if (g_squares[a].owner == X &&
				g_squares[b].owner == X &&
				g_squares[c].owner == X) {
				MessageBox(hWnd, TEXT("X takes the victory!"), TEXT("Game over"), MB_OK);
				isGameOver = TRUE;
				break;
			}
		}

		// // draw?
		if (isGameOver == FALSE) {
			isGameOver = TRUE;
			for (SIZE_T i{}; i < g_squares.size(); ++i) {
				if (g_squares[i].owner == 0/*NULL*/) {
					isGameOver = FALSE;
				}
			}
		}

		// reset
		if (isGameOver) {
			for (SIZE_T i{}; i < g_squares.size(); ++i) {
				g_squares[i].owner = 0/*NULL*/;
			}
		}

		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);

		ReleaseDC(hWnd, hDC);
		return 0;
	}

	case WM_PAINT: {
		PAINTSTRUCT ps{};
		BeginPaint(hWnd, &ps);
		FillRect(
			ps.hdc, &ps.rcPaint,
			reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH))
		);

		RECT rcClient{};
		GetClientRect(hWnd, &rcClient);

		HPEN hBlackPen{ CreatePen(PS_SOLID, 1, RGB(0, 0, 0)) };
		SelectObject(ps.hdc, hBlackPen);

		// margin
		constexpr INT MARG{ 3 };

		for (INT i{ 1 }; i <= 2; ++i) {
			MoveToEx(ps.hdc, 20 * i, 0 + MARG, NULL);
			LineTo(ps.hdc, 20 * i, 60 - MARG);
		}

		for (INT i{ 1 }; i <= 2; ++i) {
			MoveToEx(ps.hdc, 0 + MARG, 20 * i, NULL);
			LineTo(ps.hdc, 60 - MARG, 20 * i);
		}

		// draw Xs and Os
		for (SIZE_T i{}; i < g_squares.size(); ++i) {
			if (g_squares[i].owner == O) {
				Ellipse(ps.hdc,
					g_squares[i].rc.left + MARG, g_squares[i].rc.top + MARG,
					g_squares[i].rc.right - MARG, g_squares[i].rc.bottom - MARG
				);
			}
			else if (g_squares[i].owner == X) {
				MoveToEx(ps.hdc, g_squares[i].rc.left + MARG, g_squares[i].rc.top + MARG, NULL);
				LineTo(ps.hdc, g_squares[i].rc.right - MARG, g_squares[i].rc.bottom - MARG);
				MoveToEx(ps.hdc, g_squares[i].rc.right - MARG, g_squares[i].rc.top + MARG, NULL);
				LineTo(ps.hdc, g_squares[i].rc.left + MARG, g_squares[i].rc.bottom - MARG);
			}
		}

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT) {
	SetProcessDPIAware();

	CONST constexpr TCHAR* MAIN_CLASS{ TEXT("MAIN") };

	WNDCLASS wc{};
	wc.hInstance = hInstance;
	wc.lpszClassName = MAIN_CLASS;
	wc.lpfnWndProc = MainProc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	RegisterClass(&wc);


	CONST INT xy{
		(GetSystemMetrics(SM_CXSCREEN) + GetSystemMetrics(SM_CYSCREEN)) / 4
	};

	CreateWindow(
		MAIN_CLASS,
		TEXT("Tic-Tac-Toe"),
		WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		xy, xy - GetSystemMetrics(SM_CYCAPTION) * 2,
		NULL,
		NULL,
		hInstance,
		NULL
	);


	MSG msg{};
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<INT>(msg.wParam);
}