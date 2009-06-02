#include "supplicant.h"
#include "utils.h"
#include "string_utils/string_utils.h"

USERDATA stUserData;
pcap_t *fp = 0;
BOOL bOnline = FALSE;


void encode(char *strConst,unsigned char *strDest,int iSize)
{
	char *temp=new char[iSize];

	int iTimes = iSize/strlen(strConst);

	for(int i=0;i<iTimes;i++)
		memcpy(temp+i*strlen(strConst),strConst,strlen(strConst));
	memcpy(temp+iTimes*strlen(strConst),strConst,iSize%strlen(strConst));


	for(int i=0;i<iSize;i++)
		strDest[i] = strDest[i]^temp[i];
	for(int i=0;i<iSize;i++)
		strDest[iSize-i-1] = strDest[iSize-i-1]^temp[i];
	delete  []temp;
}

unsigned long _rand(unsigned long t)
{
	t = t * 0x343fd;
	t = t + 0x269ec3;
	//_asm
	//{
	//  mov eax,t
	//  sar eax,0x10
	//  and eax,0x7fff
	//}
	return t;
}


BOOL GetNIC( int nicIndex )
{
	BOOL ret = FALSE;

	//��ȡ�豸�б�
	pcap_if_t *alldevs = 0;
	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	if (pcap_findalldevs(&alldevs, errbuf) == -1) {
		utils::MyH3CError(L"error: ��ȡ������Ϣʧ��,��ȷ�ϰ�װ��winpcap������!");
		return ret;
	}
	//�õ�ÿ��������������Ϣ
	int i=0;
	for(pcap_if_t * d = alldevs; d; d = d->next)
	{
		if( i == nicIndex )
		{
			for(pcap_addr_t * a=d->addresses;a;a=a->next)
			{
				if(a->addr && (a->addr->sa_family == AF_INET) )
				{
					unsigned long uIP = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
					memcpy( stUserData.Ip,(unsigned char *)&uIP,4);
					memcpy( stUserData.nic,d->name,strlen(d->name)+1);
					utils::MyH3CError(L"message: ��������:"+string_utils::mbstowcs(std::string(d->name)));
					break;
				}
			}
			break;
		}
		i++;
	}

	pcap_freealldevs(alldevs);

	//winpcap Ŀǰû�еõ�mac��ͨ��api
	IP_ADAPTER_INFO AdapterInfo[8];
	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = ::GetAdaptersInfo(AdapterInfo,&dwBufLen);

	//���Mac
	std::string adapterName = stUserData.nic;
	adapterName = adapterName.substr( adapterName.find( '{' ) );

	for ( IP_ADAPTER_INFO* pAdapterInfo = AdapterInfo; pAdapterInfo!=NULL; pAdapterInfo=pAdapterInfo->Next )
	{
		if ( pAdapterInfo->AdapterName == adapterName )
		{
			memcpy( stUserData.Mac, pAdapterInfo->Address, sizeof(stUserData.Mac) );
			ret = TRUE;
			break;
		}
	}

	return ret;
}


void GetMacAddrFromIP(const char *strIP,unsigned char *Mac)
{
	//����Ϊ�õ�MAC��ַ�Ĵ���
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if(!memcmp(pAdapter->IpAddressList.IpAddress.String,strIP,strlen(strIP))) {
				memcpy(Mac,(char *)&pAdapter->Address,pAdapter->AddressLength);
				break;
			}
			pAdapter = pAdapter->Next;
		}
	} else {
		utils::MyH3CError(L"error: Call to GetAdaptersInfo failed.\n");
	}
	free(pAdapterInfo);
}

// �жϰ����͵ı�־
unsigned char PType[] = {0x88, 0x8e, 0x01, 0x00};

unsigned char SessionFlagA[] = {0x00, 0x05, 0x01};
unsigned char SessionFlagB[] = {0x00, 0x05, 0x01};
unsigned char SessionFlagC[] = {0x00, 0x05, 0x14};

unsigned char VersionFlag[] = {0x00, 0x05, 0x02};
unsigned char RequestPwdFlagA[] = {0x00, 0x16, 0x01};
unsigned char RequestPwdFlagB[] = {0x00, 0x16, 0x04};

unsigned char SuccessFlagA[] = {0x00, 0x04, 0x03};
unsigned char SuccessFlagB[] = {0x00, 0x04, 0x00};

unsigned char ByeFlagA[] = {0x00, 0x06, 0x04};
unsigned char ByeFlagB[] = {0x00, 0x07, 0x08};

// �����ǳ�ʼ�ĸ��ְ�
unsigned char ConnectBuf[60] = {        // ��������İ�(����Ŀ��Ϊ��Ϊ���鲥��ַ01-80-c2-00-00-03)
	0x01, 0x80, 0xc2, 0x00, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

unsigned char DisconnectBuf[60] = {    // �Ͽ�����İ�����ͬ�İ汾�᲻ͬ
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

/*
unsigned char VersionBuf[67] = {        // Э�̰汾��,��ͬ�İ汾�᲻ͬ
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00, 
0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
0x16, 0x2e, 0x25, 0x4d, 0x3b, 0x5f, 0x43, 0x5f,  
0x5d, 0x40, 0x5d, 0x5f, 0x5e, 0x5c, 0x6d, 0x6d, 
0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x02, 0x16, 0x5f,  
0x59, 0x55, 0x5e, 0xbb, 0x5c, 0x54, 0x58, 0x5a, 
0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 
0x6d, 0x6d, 0x6d  
}; 
*/


unsigned char VersionBuf[67] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00,
	0x00, 0x31, 0x02, 0x01, 0x00, 0x31, 0x02, 0x01,
	0x16,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x02, 0x16, 0x3a,
	0x71, 0x38, 0x01, 0x0b, 0x3b, 0x7e, 0x3d, 0x26,
	0x7c, 0x7c, 0x17, 0x0b, 0x46, 0x08, 0x32, 0x32,
	0x08, 0x46, 0x0b
};

unsigned char PasswdBuf[60]    = {        // ��������İ�
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00,
	0x00, 0x1c, 0x02, 0x00, 0x00, 0x1c, 0x04, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

unsigned char SessionBuf[60] = {        // ά�ֶԻ��İ�
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00,
	0x00, 0x15, 0x02, 0x00, 0x00, 0x15, 0x14, 0x00,
	0x15, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

unsigned char UsernameBuf[60] = {    // �����û����İ�
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x8e, 0x01, 0x00,
	0x00, 0x15, 0x02, 0x00, 0x00, 0x15, 0x01, 0x15,
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};



void FillDestMac(const unsigned char *strDstMAC)
{
	memcpy(VersionBuf, strDstMAC, 6);
	memcpy(UsernameBuf, strDstMAC, 6);
	memcpy(PasswdBuf,strDstMAC, 6);
	memcpy(SessionBuf, strDstMAC, 6);
	memcpy(DisconnectBuf, strDstMAC, 6);
}


void FillSrcMac(const  unsigned char *strSrcMAC)
{
	memcpy(ConnectBuf + 0x06, strSrcMAC, 6);
	memcpy(VersionBuf + 0x06, strSrcMAC, 6);
	memcpy(UsernameBuf + 0x06, strSrcMAC, 6);
	memcpy(PasswdBuf + 0x06, strSrcMAC, 6);
	memcpy(SessionBuf + 0x06, strSrcMAC, 6);
	memcpy(DisconnectBuf + 0x06, strSrcMAC, 6);
}

void FillIP(const unsigned char *strIP)
{
	memcpy(UsernameBuf + 0x19, strIP, sizeof(strIP));
	memcpy(SessionBuf + 0x1a, strIP, sizeof(strIP));
}

void FillUserName(const char *strUsername)
{
	memcpy(SessionBuf + 0x1e, strUsername, strlen(strUsername));
	memcpy(UsernameBuf + 0x1d, strUsername, strlen(strUsername));
	memcpy(PasswdBuf + 0x28, strUsername, strlen(strUsername));

	//���ó���ֵ
	SessionBuf[0x11] = strlen(strUsername) + 0x0b;
	UsernameBuf[0x11] = strlen(strUsername) + 0x0b;
	PasswdBuf[0x11] = strlen(strUsername) + 0x16;

	SessionBuf[0x15] = strlen(strUsername) + 0x0b;
	UsernameBuf[0x15] = strlen(strUsername) + 0x0b;
	PasswdBuf[0x15] = strlen(strUsername) + 0x16;
}

void FillSessionID(const unsigned char *strSessionID)
{
	memcpy(SessionBuf + 0x13, strSessionID, 1);
}

void FillUserNameID(const unsigned char *strUserNameID)
{
	memcpy(UsernameBuf + 0x13, strUserNameID, 1);
}

void FillPasswdID(const unsigned char *strUserNameID)
{
	memcpy(PasswdBuf + 0x13, strUserNameID, 1);
}


void SetMd5Buf(const u_char *ID, const u_char *chap)
{
	static u_char TmpBuf[1 + 64 + 16];
	static md5_wrapper md5T;
	static u_char digest[16];
	memcpy(TmpBuf + 0x00, ID, 1);
	memcpy(TmpBuf + 0x01, stUserData.password, strlen(stUserData.password));
	memcpy(TmpBuf + 0x01 + strlen(stUserData.password), chap, 16);
	md5T.Update(TmpBuf, 17 + strlen(stUserData.password));

	md5T.Final(digest);
	memcpy(PasswdBuf + 0x18, digest, 16);
}

BOOL Connect()
{
	if( fp == NULL )
		return FALSE;
	////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"��ʼ��֤...\n���ڲ��ҽ��뽻����...");
	//if(//SendDlgItemMessage(hDlgWnd, IDC_BOARDCAST, BM_GETCHECK, 0, 0) == BST_CHECKED) {
	//    memset(ConnectBuf, 0xff, 6);
	//  }
	utils::MyH3CError(L"message: ���Է����û���������...");
	return !pcap_sendpacket(fp, ConnectBuf, 60);
}

BOOL DisConnect()
{
	if( fp == NULL )
		return FALSE;
	////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"���ڶϿ�����...\n");
	bOnline = FALSE;
	return !pcap_sendpacket(fp, DisconnectBuf, 60);
}

BOOL OpenDevice( const char *device)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	if ((fp = pcap_open_live(device, 200, 0, 20, errbuf)) == NULL) {
		utils::MyH3CError(L"error: �������豸ʧ��");
		return FALSE;
	}
	return TRUE;
}

BOOL CloseDevice()
{
	if( fp == NULL)
		return FALSE;
	pcap_close(fp);
	return TRUE;
}


BOOL StartSupplicant()
{
	FillSrcMac(stUserData.Mac);
	FillIP(stUserData.Ip);
	FillUserName(stUserData.username);

	//�������豸
	OpenDevice(stUserData.nic);

	//�������������
	if( !Connect())
		return FALSE;

	//��ʼ�������ѭ��
	pcap_loop(fp, 0, packet_handler, NULL);
	return TRUE;
}


void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	static BOOL bFirstPacket = TRUE;
	//ȷ��������������802.1x�İ�
	if(!memcmp(pkt_data+0x0c,PType,4) && !memcmp(pkt_data+0x00,stUserData.Mac,6))
	{
		// ��õ�һ���������еõ��Ժ�������Ҫ�ظ���MAC��ַ
		if( bFirstPacket ) {
			////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"first packet\n");
			FillDestMac(pkt_data + 0x06);
			bFirstPacket = FALSE;
		}

		// �ж��Ƿ�Ϊ�Ի�ά�ְ���Ҫ�����û����İ�(�����ְ���������һ����)
		if (!memcmp(pkt_data + 0x10, SessionFlagA, 3) &&
			(!memcmp(pkt_data + 0x14, SessionFlagB, 3) ||
			!memcmp(pkt_data + 0x14, SessionFlagC, 3)) ) {
				// ������͹��û������ͶԻ�ά�ְ�
				//if (UserSended)
				if (bOnline)//�Ѿ����ߣ����ͶԻ�ά�ְ�
				{
					FillSessionID(pkt_data + 0x13);
					pcap_sendpacket(fp, SessionBuf, 60);
				} else    // ����ͷ����û���
				{
					////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"username packet\n");
					FillUserNameID(pkt_data + 0x13);
					pcap_sendpacket(fp, UsernameBuf, 60);
				}
				return;
		}


		// �ж��Ƿ�ΪЭ�̰汾��
		if (!memcmp(pkt_data + 0x10, SessionFlagA, 3) &&
			!memcmp(pkt_data + 0x14, VersionFlag, 3)) {
				////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"version packet\n");
				// ���Ͷ�Ӧ�Ļظ���(������������ǹ̶���,�汾Э��)

				encodeVersion();
				pcap_sendpacket(fp, VersionBuf, 67);
				return;
		}


		// �ж��Ƿ�ΪҪ��������İ�
		if (!memcmp(pkt_data + 0x10, RequestPwdFlagA, 3) &&
			!memcmp(pkt_data + 0x14, RequestPwdFlagB, 3)) {
				////SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"password packet...\n");
				FillPasswdID(pkt_data + 0x13);
				SetMd5Buf(pkt_data + 0x13, pkt_data + 0x18);
				pcap_sendpacket(fp, PasswdBuf, 60);
				return;
		}

		// �ж��Ƿ�Ϊ��֤�ɹ��İ� code=3
		if (!memcmp(pkt_data + 0x10, SuccessFlagA, 3) &&
			!memcmp(pkt_data + 0x14, SuccessFlagB, 3)) {
				//SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"��֤�ɹ�!\n");
				bOnline = TRUE;

				utils::MyH3CError(L"message: ��֤�ɹ���");
				utils::ReleaseAllDHCPAddr();
				// ����DHCPˢ��IP��ַ
				if(!utils::RenewAllDHCPAddr())
				{
					utils::MyH3CError(L"warning: �޷�ˢ��IP��ַ���Ƿ������˾�̬��ַ��");
				}
				else
				{
					utils::MyH3CError(L"message: ��ˢ��IP��ַ��");
				}

				return;
		}

		// ��֤ʧ�ܻ����߰� code=4
		if (!memcmp(pkt_data + 0x10, ByeFlagA, 3)) {
			// �ж��Ƿ�Ϊ���߳ɹ��İ�
			if(!memcmp(pkt_data + 0x14, ByeFlagB, 3)) {
				bOnline = FALSE;
				utils::MyH3CError(L"message: �����ߡ�");
				//SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)"�ɹ�����\n");
			} else// �������߰�,���������ԭ��
			{
				utils::MyH3CError(L"message: �Ͽ����ӡ�");
				//SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)(pkt_data+0x18));
			}

			//EnableWindow(GetDlgItem(hDlgWnd,IDC_SUPPLICANT),TRUE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_DISCONNECT),FALSE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_USERNAME),TRUE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_PASSWORD),TRUE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_IPADDRESS),TRUE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_MAC),TRUE);
			//EnableWindow(GetDlgItem(hDlgWnd,IDC_NICS),TRUE);

			//ֻ��3.1���ϵİ汾�в���pcap_breakloop
			//typedef void (*PCAP_LOOP)(pcap_t *);
			//PCAP_LOOP my_pcap_loop;
			//my_pcap_loop = (PCAP_LOOP)GetProcAddress(LoadLibrary("wpcap.dll"), "pcap_breakloop");
			//if(my_pcap_loop != NULL) {
			pcap_breakloop(fp);
			CloseDevice();
			//}
			return;
		}
		//��ʾ��֤�ɹ���Ϣ
		if(*(pkt_data +0x12) == 0x0a && *(pkt_data +0x1a) == 0xc4
			&& *(pkt_data +0x1b) == 0xfa) {
				u_char a[0x100] = {0};
				memcpy(a, pkt_data+0x1a, *(pkt_data +0x11)-4);
				a[*(pkt_data +0x11)-3] = '\0';
				for(int i= 0; i<0x100; i++) {
					if((a[0x100-i] == 0x34) && (a[0x100-i+1] == 0x86)) {
						a[0x100-i] = '\n';
						a[0x100-i+1] = '\n';
						break;
					}
				}
				//SendDlgItemMessage(hDlgWnd,IDC_MSG,WM_SETTEXT,0,(LPARAM)a);
		}
	}
}

void encodeVersion()
{
	unsigned long uRand = _rand((unsigned long)time(NULL));
	unsigned long magic = uRand * GetTickCount();

	//��ʼ��strMagic,��magic��16������ʽת��Ϊ�ַ����������
	char strMagic[9]={0};
	unsigned char strTemp[4] = {0};
	memcpy(strTemp,(unsigned char *)&magic,4);
	sprintf(strMagic,"%02x%02x%02x%02x",strTemp[0],strTemp[1],strTemp[2],strTemp[3]);
	//��ʼ���汾��Ϣ
	unsigned char version[20];
	memset(version,0,sizeof(version));
	memcpy(version,"CH V2.20-0247",strlen("CH V2.20-0247"));
	memcpy(version+16,(unsigned char *)&magic,4);

	encode(strMagic,version,0x10);

	encode("HuaWei3COM1X",version,0x14);
	memcpy(VersionBuf+25,version,20);
}