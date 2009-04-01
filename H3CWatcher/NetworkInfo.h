#pragma once
#include "stdafx.h"
#include <exception>

//Get network infos.
class NetworkInfo
{
public:
	typedef int AdpaterIdentifier;		//TODO: replace int with actual adpater identifier

	class InvalidIdentifier : public runtime_error{
	public:
		InvalidIdentifier(const char* msg):runtime_error(msg){}
	};

	//adpaterPos: ���ݸ�����д��pw.data�еġ��ڼ�����������
	static void EnumerateAdapters(
		const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter )
	{
		//TODO: implement this
		onGotAdapter(1, 1, L"Test 1");
		onGotAdapter(2, 2, L"Test 2");
		onGotAdapter(3, 3, L"Test 3");
	}

	static string GetGatewayAddress( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
		return "";
	}

	static void EnableConnection( const AdpaterIdentifier& adapterId, bool enabled )
	{
		//TODO: implement this
	}
};