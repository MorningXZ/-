#include <tchar.h>
#include "CWndBase.h"
#include "resource.h"
#include "CBubble.h"
#include "CBubbleManger.h"
#include <gdiplusbrush.h>
#define MSG_UNDONE 0
#define MSG_DONE 1
#define className "WindowBase"

CWndBase::CWndBase(HINSTANCE hInstance) :m_hInstance(hInstance), m_hWnd(nullptr)
{
	registerWindow();
	create();
	m_pBubbleManger = new CBubbleManger(10);
}

CWndBase::~CWndBase()
{

}

bool CWndBase::registerWindow(){
	WNDCLASSEX wcex;
	if (GetClassInfoEx(m_hInstance, _T(className), &wcex))
	{
		return true;
	}

	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));//(HBRUSH)(COLOR_WINDOW + 2);
	
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T(className);
	wcex.hIconSm = NULL;

	if (0 == ::RegisterClassEx(&wcex))
	{
		return false;
	}else{
		return true;
	}
}

bool CWndBase::create()
{
	if (this->m_hWnd == nullptr)
	{
		DWORD dwStyle = WS_POPUP|WS_OVERLAPPED;
		HWND hwnd = CreateWindow(_T(className),
								_T(className),
								dwStyle,
								0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
								NULL,
								NULL,
								m_hInstance,
								(LPVOID)this);

		if (hwnd == NULL)
		{
			this->m_hWnd = NULL;
			wchar_t msg[100];
			::wsprintf(msg, _T("CreateWindow()ʧ��:%ld"), ::GetLastError());
			::MessageBox(NULL, msg, _T("����"), MB_OK);
			return false;
		}
		m_hWnd = hwnd;
		LONG nRet = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		nRet |= WS_EX_LAYERED;
		SetWindowLong(m_hWnd, GWL_EXSTYLE, nRet);
		//SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY|LWA_ALPHA);
		return true;
	}
	return true;
}

void CWndBase::show(bool isModal, CWndBase* parentWnd){
	if (isModal && parentWnd)//�����ģ̬��������ø�����
	{
		EnableWindow(parentWnd->hWnd(), false);
	}
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}


LRESULT CWndBase::OnDestory(HWND hWnd, WPARAM wParam, LPARAM lParam){
	// ��ʱ�����ڶ����Ѿ����٣�ͨ������hWnd=NULL����֪ͨC++����
	PostQuitMessage(0);
	m_hWnd = NULL;
	return MSG_DONE;
}


LRESULT CWndBase::OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PostMessageW(hWnd, WM_PAINT, NULL, NULL);
	m_pBubbleManger->move();
	return MSG_DONE;
}

LRESULT CWndBase::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	RECT rect;
	GetClientRect(hWnd, &rect);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);	
	HDC m_hdcMem = CreateCompatibleDC(hdc);
	HBITMAP m_hBitmap = CreateCompatibleBitmap(hdc, rect.right-rect.left, rect.bottom-rect.top); //��������Ļ���ݵ�bitmap    
	HBITMAP m_hOldBitmap = (HBITMAP)SelectObject(m_hdcMem, m_hBitmap);//��memBitmapѡ���ڴ�DC   
	m_pBubbleManger->show(m_hdcMem);

	BLENDFUNCTION _Blend;
	_Blend.BlendOp = 0;
	_Blend.BlendFlags = 0;
	_Blend.AlphaFormat = 1;
	_Blend.SourceConstantAlpha = 255;
	POINT pt = { 0, 0};
	SIZE sz = { rect.right - rect.left, rect.bottom - rect.top };
	UpdateLayeredWindow(m_hWnd, hdc, &pt, &sz, m_hdcMem, &pt, 0, &_Blend, ULW_ALPHA);

	//BitBlt(hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, m_hdcMem, rect.left, rect.top, SRCCOPY);
	EndPaint(hWnd, &ps);
	SelectObject(m_hdcMem, m_hOldBitmap);
	DeleteObject(m_hBitmap);
	DeleteDC(m_hdcMem);

	return 0;
}

int CWndBase::preDispatchMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
			return MSG_DONE;
		}
		return MSG_UNDONE;
		
	default:
		return MSG_UNDONE;
	}
}


int CWndBase::dispatchMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
	case WM_DESTROY:
		return OnDestory(hWnd, wParam, lParam);

	//case WM_MOVE:
	//	return OnMove(hWnd, wParam, lParam);

	case WM_PAINT:
		return OnPaint(hWnd, wParam, lParam);

	case WM_TIMER:
		return OnTimer(hWnd, wParam, lParam);

	default:
		return MSG_UNDONE;//��Ϣδ������Ĭ�Ϻ���ȥ����
	}		
}


LRESULT CALLBACK CWndBase::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	// �ڴ���Ϣ�յ�ʱ���Ѵ��ڶ���������C++�����Ա
	// ͬʱ��C++�����ַ�������ڶ����Ա
	CWndBase* pWnd = NULL;
	if (message == WM_CREATE){
		pWnd = (CWndBase*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		pWnd->m_hWnd = hWnd;	                                         // �ڴ˴���ȡHWND����ʱCreateWindow()��δ���ء�
		::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pWnd);             // ͨ��USERDATA��HWND��C++�����������
		PostMessageW(hWnd, WM_PAINT, NULL, NULL);
	}

	pWnd = (CWndBase*)::GetWindowLong(hWnd, GWL_USERDATA);
	if (pWnd!=nullptr)
	{
		//��Ϣ��ǰ���أ����������������Ӧ
		if (MSG_DONE == pWnd->preDispatchMessage(hWnd, message, wParam, lParam))
		{
			return MSG_DONE;
		}

		if (MSG_DONE == pWnd->dispatchMessage(hWnd, message, wParam, lParam))
		{
			return MSG_DONE;	
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}else{//ע�������߼��ȽϹؼ��������޷���������
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
