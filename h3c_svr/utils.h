/////////////////////////
// ���������¼������ܣ�
// 1.���������Ϣ: utils::MyH3CError(const T& errArg)
// 2.�ͷ�DHCP��ַ: utils::ReleaseDHCPAddr(int adapterID)
// 3.����DHCP��ַ: utils::RenewDHCPAddr(int adapterID)

#pragma once
#include <string>
#include <sstream>
#include <fstream>
//#include <Windows.h>

namespace utils{

	//void SetLogFileName(const wchar_t* filename);
	std::wstring GetLogFileName();

	/**
	 * ���������Ϣ��
	 * @param errMsg ����
	 */
	void MyH3CError(const std::wstring& errMsg);

	/**
	 * �ͷŵ�ǰ��DHCP��������õ�IP��ַ��
	 * @param adapterID ����ID��
	 * @return �����Ƿ�ɹ���
	 */
	bool ReleaseDHCPAddr(int adapterID);

	/**
	 * ��DHCP�������������IP��ַ��
	 * @param adapterID ����ID��
	 * @return �����Ƿ�ɹ���
	 */
	bool RenewDHCPAddr(int adapterID);
	
	/**
	 * ������������Ӧ�����DHCP�������������IP��ַ��
	 * @return ֻҪ��ĳһ���������ɹ�������true����һ�ɹ��򷵻�false��
	 */
	bool ReleaseAllDHCPAddr();

	/**
	 * ������������Ӧ�����DHCP�������������IP��ַ��
	 * @return ֻҪ��ĳһ���������ɹ�������true����һ�ɹ��򷵻�false��
	 */
	bool RenewAllDHCPAddr();

	/**
	 * ͨ�������������ص�80�˿ڣ���鱾���Ƿ����ߡ�
	 * @return �����ӳɻ򣬷��ػ�true�����򷵻�false��
	 */
	bool CheckOnline(const std::string& adapterName);

};
