
#include "CDuiApp.h"
#include "CWndBase.h"
#include "public.h"

HINSTANCE CDuiApp::m_hInstance = 0;

CDuiApp::CDuiApp(HINSTANCE hInstance) 
{
	m_hInstance = hInstance;

	//1.��ʼ��
	init();
}


CDuiApp::~CDuiApp()
{
}

void CDuiApp::init(){
	//��ʼ��gdi+��Դ
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

void CDuiApp::release(){
	//�ͷ�gdi+��Դ
	GdiplusShutdown(m_gdiplusToken);
}

void CDuiApp::run()
{
	//2.������Ϣѭ��
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//3.�ͷ���Դ
	release();
	return;
}

HINSTANCE CDuiApp::hInstance(){
	return m_hInstance;
}
