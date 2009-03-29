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
		function<void(AdpaterIdentifier/*id*/, int /*adpaterPos*/, const wstring&/*name*/)>& onGotAdapter )
	{
		//TODO: implement this
	}

	static string GetGatewayAddress( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
		return "";
	}

	static void DisableConnection( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
	}
	
	static void EnableConnection( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
	}
};