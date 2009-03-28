#define _vsnprintf vsnprintf
#include <pcap.h>
#include <iphlpapi.h>
#include "md5_wrapper.h"

#include "resource.h"

#define PCAP_OPENFLAG_PROMISCUOUS 1
//�����û���Ϣ�ṹ
typedef struct UserData
{
	char username[20];
	char password[20];
	unsigned char Ip[4];
	unsigned char Mac[6];
	char nic[60];
}USERDATA;

void GetNIC( int nicIndex);

void GetMacAddrFromIP(const char *strIP,unsigned char *Mac);


void FillDestMac(const unsigned char *strDstMAC);    // �����ְ���Destination MAC 
void FillSrcMac(const  unsigned char *strSrcMAC);    // �����ְ���Source MAC

void FillIP(const unsigned char *strIP);
 
void FillUserName(const char *strUsername);


void FillUserNameID(const unsigned char *strUserNameID);  // ��䷢���û�������ID
void FillSessionID(const unsigned char *strSessionID);   // ���Ի�ά�ְ���ID
void FillPasswdID(const unsigned char *strUserNameID);   // ��䷢���������ID


void SetMd5Buf(const u_char *ID, const u_char *chap);    //MD5 �����㷨,�������ܺ�Ļظ�����


BOOL Connect();
BOOL DisConnect();

BOOL OpenDevice( const char *device);
BOOL CloseDevice();

BOOL StartSupplicant(); 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

void encodeVersion();
