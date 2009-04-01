#pragma once
#include "Menu.h"
#include "H3CWatcher.h"
#include "NetworkInfo.h"
#include <sstream>

class FunctionMenu : public Menu
{
	Menu selAdptMenu;
	Settings* setting;
	HWND hMainWnd;
	MenuItem::CmdFunc onAbout;
	Restarter* restarter;
public:
	FunctionMenu(HWND hMainWnd_, Restarter* restarter_, Settings* setting_,
		const MenuItem::CmdFunc& onAbout_, const AdapterMap& adapters)
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

		//ö����������̬���ɲ˵���
		UINT pos = 0;
		for(AdapterMap::const_iterator iter = adapters.begin(), end=adapters.end();
			iter!=end; iter++, pos++)
		{
			wostringstream strm;
			strm<<L"("<<iter->second.adapterPos<<L") "<<iter->second.name;
			selAdptMenu.Append( &MenuItem( const_cast<wchar_t*>( strm.str().c_str() ),
				bind( &FunctionMenu::OnSelectAdapter, this, iter->first, pos ) ) );
			//Check the menu item.
			if( iter->first==setting->adapterId )
				selAdptMenu.CheckRadioItem( pos );
		}
	}

	void OnSelectAdapter( const NetworkInfo::AdpaterIdentifier& id, UINT pos )
	{
		setting->adapterId = id;
		ChangeSettingsAndRestart( *setting );
		selAdptMenu.CheckRadioItem( pos );
	}

	void Restart()
	{
		restarter->TryRestart();
	}

	void DisableCurConn()
	{
		try
		{
			NetworkInfo::EnableConnection( setting->adapterId, false );
		}
		catch(const NetworkInfo::InvalidIdentifier& err)
		{
			ShowErrorMessage(err.what());
		}
	}
	
	void EnableCurConn()
	{
		try
		{
			NetworkInfo::EnableConnection( setting->adapterId, true );
		}
		catch(const NetworkInfo::InvalidIdentifier& err)
		{
			ShowErrorMessage(err.what());
		}
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

	void ShowErrorMessage(const char* msg)
	{
		MessageBoxA(hMainWnd, msg, "Error", MB_OK|MB_ICONERROR);
	}
};