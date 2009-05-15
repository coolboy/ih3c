/* Copyright (c) 2009
 * Subject to the GPLv3 Software License. 
 * (See accompanying file GPLv3.txt or http://www.gnu.org/licenses/gpl.txt)
 * Author: Xiao, Yang
 */

#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <memory>
#include "ServiceController.h"

enum RestarterMessageType
{
	RestarterMessageTypeRestarting,
	RestarterMessageTypeNoServiceControllingRight,
	RestarterMessageTypeConnectionSucceeded,
};

class Restarter
{
public:
	typedef function<void(RestarterMessageType, const wstring&, const wstring&)> MsgFunc;
	string host;
	int port;
protected:
	auto_ptr<thread> pcheckthrd;
	MsgFunc onMessage;
	time_duration checkInterval, restartInterval;
	bool lastTimeConnectable;
	ptime restartTime;
public:
	Restarter(const time_duration& checkInterval_, const string& host_,
		int port_, const MsgFunc& onMessage_)
		:host(host_), port(port_), lastTimeConnectable(false),
		onMessage(onMessage_), checkInterval(checkInterval_){}
	  
	~Restarter()
	{
		pcheckthrd.reset();
	}

	void Start()
	{
		pcheckthrd.reset(new thread(bind(&Restarter::Check, this)));
	}

	void TryRestart(const wstring& msg = L"H3C������������.....")
	{
		if(onMessage)
			onMessage( RestarterMessageTypeRestarting, msg, L"H3C��������");
		ServiceController sc;
		if(!sc.RestartService(L"MyH3C"))
		{
			if(onMessage)
				onMessage( RestarterMessageTypeNoServiceControllingRight, 
				L"����������MyH3C����Ĺ����г��ִ��󡣿����ǣ�\r\n"
				L"  > �޷�ȡ�öԷ���Ŀ���Ȩ���볢���Թ���Ա��������д˳���\r\n"
				L"  > ��δ�ɹ���װMyH3C�������Թ���Ա���������h3c_svr.exe -i��\r\n",
					L"�޷�����MyH3C����");
		}
	}

	ptime GetLastRestartTime() const
	{
		return restartTime;
	}

protected:
	void RestartMyH3C(const wstring& msg)
	{
		TryRestart(msg);
	}

	class ConnectWithTimeout
	{
	public:
		ConnectWithTimeout()
			: timer_(io_service_), socket_(io_service_), succeeded(false)
		{
		}

		void run(const string& addr, int port, const time_duration& timeout)
		{
			socket_.async_connect(
				ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(addr), port),
				boost::bind(&ConnectWithTimeout::handle_connect, this,
				boost::asio::placeholders::error));

			timer_.expires_from_now(timeout);
			timer_.async_wait(boost::bind(&ConnectWithTimeout::close, this));
			io_service_.run();
		}

		void handle_connect(const boost::system::error_code& err)
		{
			succeeded = !(err);
			close();
		}

		void close()
		{
			socket_.close();
		}

		bool connection_succeeded() const
		{
			return succeeded;
		}
	
	private:  
		io_service io_service_;  
		deadline_timer timer_;  
		ip::tcp::socket socket_;
		bool succeeded;
	};

	void Check()
	{
		while(true)
		{
			ConnectWithTimeout cwt;
			cwt.run(host, port, seconds(2));

			if (cwt.connection_succeeded())
			{
				if(!lastTimeConnectable)
				{
					if(onMessage)
						onMessage( RestarterMessageTypeConnectionSucceeded,
							L"�Ѿ��ɹ�ͨ��H3C���ӵ����硣", L"��������������");
					lastTimeConnectable = true;
				}
				this_thread::sleep(checkInterval);
			}

			if ( !cwt.connection_succeeded() )
			{
				ptime now = microsec_clock::local_time();
				if ( lastTimeConnectable || restartTime.is_not_a_date_time() || now-restartTime>seconds(15) )
				{
					lastTimeConnectable = false;
					restartTime = microsec_clock::local_time();
					RestartMyH3C(L"����ָ��ʱ��δ���������أ�H3C���ӿ����ѶϿ���\r\nH3C������������.....");
				}
			}
		}
	}

};
