#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <windowsx.h>
#include <future>
#include <vector>
#include <algorithm>
#include "TableReader.h"

#define OPEN_FILE_ID 700
char file_path[256] = "\0";
OPENFILENAME ofn;
using StringTable = std::vector<std::vector<std::string>>;
StringTable strings;
std::vector<size_t> max_row_heights;
size_t CLIENT_AREA_WIDTH;
size_t CLIENT_AREA_HEIGHT;

void initialize_ofn_structure(HWND hWnd) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = file_path;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(file_path);
	ofn.lpstrFilter = "Таблицы\0*.txt\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
}

void set_open_file_menu(HWND hWnd) {
	HMENU hMenuBar = CreateMenu();
	HMENU hFile = CreateMenu();
	AppendMenu(hMenuBar, MF_POPUP, (UINT)hFile, "Файл");
	AppendMenu(hFile, MF_STRING, (UINT_PTR)OPEN_FILE_ID, "Открыть");
	SetMenu(hWnd, hMenuBar);
}

void DrawTableText(HDC hDC) {
	if (CLIENT_AREA_WIDTH < 20) return;
	if (CLIENT_AREA_HEIGHT < 20) return;
	const size_t rows = strings.size();
	const size_t columns = strings[0].size();
	const LONG column_width = CLIENT_AREA_WIDTH / columns;
	max_row_heights.resize(rows);

	size_t font_size = 1;
	RECT rect = { 0, 0, column_width, column_width };
	HFONT hFont;
	static LOGFONT lf;
	lf.lfHeight = font_size;
	hFont = CreateFontIndirect(&lf);
	HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

	bool biggest_font_is_not_found = true;

	while (biggest_font_is_not_found) {

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < columns; ++j) {
				rect = { 0, 0, column_width, column_width };
					DrawText(hDC, strings[i][j].c_str(), -1, &rect, DT_WORDBREAK | DT_LEFT | DT_CALCRECT);
					size_t calculated_width = rect.right - rect.left;
					if (calculated_width > column_width) { biggest_font_is_not_found = false; break; }
			}

			if (!biggest_font_is_not_found) break;
		}

		if (!biggest_font_is_not_found) break;
		
		font_size += 1;
		lf.lfHeight = font_size;
		DeleteObject(hFont);
		hFont = CreateFontIndirect(&lf);
		SelectObject(hDC, hFont);
	}

	font_size -= 1;
	hFont = (HFONT)SelectObject(hDC, hOldFont);
	if (hFont) DeleteObject(hFont);
	lf.lfHeight = font_size;
	hFont = CreateFontIndirect(&lf);
	hOldFont = (HFONT)SelectObject(hDC, hFont);

	std::vector<size_t> heights(columns);

	while (true) {

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < columns; ++j) {
				rect = { 0, 0, column_width, column_width };
				DrawText(hDC, strings[i][j].c_str(), -1, &rect, DT_WORDBREAK | DT_LEFT | DT_CALCRECT);
				size_t calculated_height = rect.bottom - rect.top;
				heights[j] = calculated_height;
			}
			sort(heights.begin(), heights.end());
			max_row_heights[i] = heights[heights.size() - 1];
		}

		size_t sum = 0;
		for (auto& it : max_row_heights) {
			sum += it;
		}

		if (sum < CLIENT_AREA_HEIGHT) break;

		font_size -= 1;
		lf.lfHeight = font_size;
		DeleteObject(hFont);
		hFont = CreateFontIndirect(&lf);
		SelectObject(hDC, hFont);
	}

	RECT cell = { 0, 0, column_width, 0 };
	for (size_t i = 0; i < rows; ++i) {
		cell.bottom += max_row_heights[i];
		for (size_t j = 0; j < columns; ++j) {
			DrawText(hDC, strings[i][j].c_str(), -1, &cell, DT_WORDBREAK | DT_LEFT);
			cell.left += column_width;
			cell.right += column_width;
		}
		cell.left = 0;
		cell.right = column_width;
		cell.top += max_row_heights[i];
	}

	hFont = (HFONT)SelectObject(hDC, hOldFont);
	if (hFont) DeleteObject(hFont);
}

void DrawLines(HDC hDC) {
	if (CLIENT_AREA_WIDTH < 20) return;
	if (CLIENT_AREA_HEIGHT < 20) return;
	size_t wholeHeight = 0;
	for (auto& it : max_row_heights) {
		wholeHeight += it;
	}
	size_t columns = strings[0].size();
	size_t rows = strings.size();
	size_t column_width = CLIENT_AREA_WIDTH / columns;
	size_t changingX = column_width;
	for (size_t i = 0; i < columns; ++i) {
		MoveToEx(hDC, changingX, 0, NULL);
		LineTo(hDC, changingX, wholeHeight);
		changingX += column_width;
	}

	size_t changingY = 0;
	for (size_t i = 0; i < rows; ++i) {
		changingY += max_row_heights[i];
		MoveToEx(hDC, 0, changingY, NULL);
		LineTo(hDC, CLIENT_AREA_WIDTH, changingY);
	}
}

void RedrawDrawAreaBackground(HDC hDC) {
	RECT rect;
	SetRect(&rect, 0, 0, CLIENT_AREA_WIDTH, CLIENT_AREA_HEIGHT);
	FillRect(hDC, &rect, (HBRUSH)(WHITE_BRUSH));
}

void DrawTable(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateCompatibleBitmap(hdc, CLIENT_AREA_WIDTH, CLIENT_AREA_HEIGHT);
	SelectObject(hMemDC, memBM);

	RedrawDrawAreaBackground(hMemDC);
	DrawTableText(hMemDC);
	DrawLines(hMemDC);


	BitBlt(hdc, 0, 0, CLIENT_AREA_WIDTH, CLIENT_AREA_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	DeleteDC(hMemDC);
	DeleteObject(memBM);
}

LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_CREATE: {
		initialize_ofn_structure(hWnd);
		set_open_file_menu(hWnd);
		return 0;
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_COMMAND: {
		WORD word = LOWORD(wParam);

		switch (word) {
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case OPEN_FILE_ID: {

			if (GetOpenFileName(&ofn) == TRUE) {
				TableReader rd;
				rd.set_file_path(file_path);
				try {
					strings = move(rd.get_file_contents());
					InvalidateRect(hWnd, NULL, TRUE);
				}
				catch(...){
					MessageBox(hWnd, "Произошла ошибка чтения!", "Ошибка!", MB_ICONERROR);
				}
			}
			else {
				MessageBox(hWnd, "Файл не был открыт!", "Внимание!", MB_ICONEXCLAMATION);
			}

			break;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		if (!strings.empty())
			DrawTable(hdc);
		else {
			RECT rect = { 0, 0, CLIENT_AREA_WIDTH, CLIENT_AREA_HEIGHT };
			RedrawDrawAreaBackground(hdc);
			DrawText(hdc, "Не был выбран файл с текстовой таблицей!", -1, &rect, DT_CENTER);
		}

		EndPaint(hWnd, &ps);
		return 0;
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_SIZE: {
		CLIENT_AREA_WIDTH  = LOWORD(lParam);
		CLIENT_AREA_HEIGHT = HIWORD(lParam);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wcex;	//Оконный класс для окна нашего приложения
	HWND hWnd;			//Главное окно нашего приложения
	MSG msg;			//Структура для получения и отправки сообщений

	//////////////////////////////////////////////
	//////////////////////////////////////////////			//Заполнение полей оконного класса
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = MyWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "MyWindowClass";
	wcex.hIconSm = NULL;
	//////////////////////////////////////////////
	//////////////////////////////////////////////

	RegisterClassEx(&wcex);		//Регистрация оконного класса

	//Создание главного окна приложения
	hWnd = CreateWindowEx(0, "MyWindowClass", "Лабораторная работа №2 по ОСиСП",
		WS_OVERLAPPEDWINDOW,
		0, 0, 800, 600, 0, 0, hInstance, NULL);


	//Показ главного окна приложения
	ShowWindow(hWnd, nCmdShow);


	//Цикл обработки сообщений
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}


	//Возвращение результата работы
	return msg.wParam;
}