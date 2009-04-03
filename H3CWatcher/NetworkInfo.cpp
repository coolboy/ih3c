#include "stdafx.h"
#include "NetworkInfo.h"
#include "string_utils.h"

#include <IPHlpApi.h>
#include <SetupAPI.h>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void NetworkInfo::EnumerateAdapters( const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter )
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}
	UINT i = 1;

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			onGotAdapter(pAdapter->AdapterName, i, mbstowcs(pAdapter->Description).c_str());
			pAdapter = pAdapter->Next;
			++i;
		}
	} else {
		throw InvalidIdentifier("GetAdaptersInfo failed with error: %d\n"/*, dwRetVal*/);
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);
}

std::string NetworkInfo::GetGatewayAddress( const AdpaterIdentifier& adapterId )
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	std::string retStr;

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}
	UINT i = 1;

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if (adapterId == pAdapter->AdapterName)
				retStr = pAdapter->GatewayList.IpAddress.String;
			pAdapter = pAdapter->Next;
			++i;
		}
	} else {
		throw InvalidIdentifier("GetAdaptersInfo failed with error: %d\n"/*, dwRetVal*/);
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return retStr;
}

void NetworkInfo::SetConnection( const AdpaterIdentifier& adapterId, bool enabled )
{
	IN LPTSTR HardwareId = L"PCI\\VEN_10EC&DEV_8139&SUBSYS_813910EC" ;
	//Ӳ��ComponentId��ע����ַ��system\currentcontrolset\class\{4D36E972-E325-11CE-BFC1-08002BE10318}\0000

	DWORD NewState ;

	if(enabled == false)
	{
		NewState = DICS_DISABLE ;
		//����
	}
	else
	{
		NewState = DICS_ENABLE ;
		//����
	}

	//����ddk����������������

	DWORD i,err ;
	BOOL Found=false ;

	HDEVINFO hDevInfo ;
	SP_DEVINFO_DATA spDevInfoData ;

	//����ϵͳ��Ӳ����
	hDevInfo=SetupDiGetClassDevs(NULL,L"PCI", NULL,DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if(hDevInfo==INVALID_HANDLE_VALUE)
	{
		//printf("����ϵͳӲ������");
		return;
	}

	//ö��Ӳ���������Ҫ�Ľӿ�
	spDevInfoData.cbSize=sizeof(SP_DEVINFO_DATA);
	for(i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&spDevInfoData);i++)
	{
		DWORD DataT ;
		LPTSTR p,buffer=NULL ;
		DWORD buffersize=0 ;

		//���Ӳ��������ֵ
		while(!SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&spDevInfoData,
			SPDRP_HARDWAREID,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if(GetLastError()==ERROR_INVALID_DATA)
			{
				//������HardwareID. Continue.
				break ;
			}
			else if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				//buffer size����.
				if(buffer)
					LocalFree(buffer);
				buffer=(LPTSTR)LocalAlloc(LPTR, buffersize*2);
			}
			else
			{
				//δ֪����
				goto cleanup_DeviceInfo ;
			}
		}

		if(GetLastError()==ERROR_INVALID_DATA)
			continue ;

		//�Ƚϣ��ҵ�������ID��ͬ����
		for(p=buffer;*p&&(p<&buffer[buffersize]);p+=lstrlen(p)+sizeof(TCHAR))
		{

			if(!_tcscmp(HardwareId,p))
			{
				//�ҵ�����
				Found=TRUE ;
				break ;
			}
		}

		if(buffer)
			LocalFree(buffer);

		//������
		if(Found)
		{
			//���ø��豸

			SP_PROPCHANGE_PARAMS spPropChangeParams ;

			spPropChangeParams.ClassInstallHeader.cbSize=sizeof(SP_CLASSINSTALL_HEADER);
			spPropChangeParams.ClassInstallHeader.InstallFunction=DIF_PROPERTYCHANGE ;
			spPropChangeParams.Scope=DICS_FLAG_GLOBAL ;
			spPropChangeParams.StateChange=NewState ;
			//���ã�DICS_DISABLE��DICS_ENABLE����

			//
			if(!SetupDiSetClassInstallParams(hDevInfo,&spDevInfoData,(SP_CLASSINSTALL_HEADER*)&spPropChangeParams,sizeof(spPropChangeParams)))
			{
				DWORD errorcode=GetLastError();
			}

			if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&spDevInfoData))
			{
				DWORD errorcode=GetLastError();
			}

			switch(NewState)
			{
			case DICS_DISABLE :
				//printf("�ɹ��������磡");
				break ;
			case DICS_ENABLE :
				//printf("�ɹ��������磡");
				break ;
			}

			break ;
		}

	}

	//�˳�ʱ������������
cleanup_DeviceInfo :
	err=GetLastError();
	SetupDiDestroyDeviceInfoList(hDevInfo);
	SetLastError(err);
}