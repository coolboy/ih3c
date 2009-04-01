#pragma once
#include "Menu.h"
#include "H3CWatcher.h"
#include "NetworkInfo.h"

class FunctionMenu : public Menu
{
	Menu selAdptMenu;
	Settings* setting;
	HWND hMainWnd;
	MenuItem::CmdFunc onAbout;
	Restarter* restarter;
public:
	FunctionMenu(HWND hMainWnd_, Restarter* restarter_, Settings* setting_,
		const MenuItem::CmdFunc& onAbout_)
		:Menu(true), selAdptMenu(true), setting(setting_), hMainWnd(hMainWnd_),
		onAbout(onAbout_), restarter(restarter_)
	{
		Append( &MenuItem( L"��������MyH3C����(&R)", bind( &FunctionMenu::Restart, this ) ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"ѡ������(&S)", MenuItem::CmdFunc(), selAdptMenu ) );
		Append( &MenuItem( L"���õ�ǰ����(&D)", bind( &FunctionMenu::DisableCurConn, this ) ) );
		Append( &MenuItem( L"���õ�ǰ����(&E)", bind( &FunctionMenu::EnableCurConn, this ) ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"����(&H)", bind( &FunctionMenu::Help, this ) ) );
		Append( &MenuItem( L"����(&A)...", onAbout ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"�˳�(&X)", bind( &FunctionMenu::Exit, this ) ) );

		//TODO:ö����������̬���ɲ˵���
	}

	void Restart()
	{
		restarter->TryRestart();
	}

	void DisableCurConn()
	{
		//TODO:
	}
	
	void EnableCurConn()
	{
		//TODO:
	}

	void Help()
	{
		//TODO:
	}

	void Exit()
	{
		if ( MessageBox( hMainWnd, L"ȷ��Ҫ�˳����˳�֮��ʧȥ�����������ܡ�", L"iH3C�˳�ȷ��",
			MB_ICONQUESTION|MB_OKCANCEL ) == IDOK )
		{
			SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		}
	}
};