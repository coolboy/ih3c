// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define NOMINMAX
#define BOOST_BIND_NO_PLACEHOLDERS
#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <boost/date_time.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <string>
#include <memory>
#include <boost/smart_ptr.hpp>

using namespace boost::posix_time;
using namespace boost::asio;
using namespace boost;
using namespace std;

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
