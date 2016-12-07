/**
 * \file zHttpClient.cpp
 * \version  $Id$
 * \author  , 
 * \date 2013年07月18日 17时40分02秒 CST
 * \brief 一个基于curl实现一个轻量级http客户端,http的连接线程池另外实现
 *
 * 
 */

#include <curl/curl.h>
#include "zHttpClient.h"
#include "Fir.h"

zHttpClient::zHttpClient(const std::string &requrl): url(requrl)
{
	state = 0;
}

zHttpClient::zHttpClient()
{
	state = 0;
}

zHttpClient::~zHttpClient()
{
	curl_easy_cleanup(m_curl);
}

int zHttpClient::init_curl()
{
	m_curl = curl_easy_init();

	if(m_curl == NULL)
	{
		Fir::logger->error("curl_easy_init failed.");
		return -1;
	}

	return 0;
}

void zHttpClient::init_requrl(const std::string& requrl)
{
	url = requrl;
}

bool zHttpClient::httpPost(std::string &ret, const std::string &postdata)
{
	ret.clear();
	char errorBuffer[CURL_ERROR_SIZE];
	reset();

	if (m_curl == NULL)
	{
		Fir::logger->error("m_curl is null.");
		return false;
	}

	//设置本次操作为POST
	curl_easy_setopt(m_curl, CURLOPT_POST, 1);
	
	curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, zHttpClient::writer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ret);
	if(postdata.length() != 0)
	{
		curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, postdata.c_str());
	}
	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());

	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(m_curl);
	if(res != CURLE_OK)
	{
		Fir::logger->error("请求%s 失败，%d，%s", url.c_str(), res, errorBuffer);
		return false;
	}

	return true;
}

bool zHttpClient::httpGet(std::string &ret, const std::string &postdata)
{
	ret.clear();
	char errorBuffer[CURL_ERROR_SIZE];
	reset();

	if (m_curl == NULL)
	{
		Fir::logger->error("m_curl is null.");
		return false;
	}

	//设置初始化
	curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);

	std::string geturl = url + "?";
	geturl += postdata;
	
	curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, zHttpClient::writer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ret);
	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(m_curl, CURLOPT_URL, geturl.c_str());

	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(m_curl);
	if(res != CURLE_OK)
	{
		Fir::logger->error("请求%s 失败，%d，%s", geturl.c_str(), res, errorBuffer);
		return false;
	}

	return true;
}

size_t zHttpClient::writer(char *data, size_t size, size_t nmemb, std::string *buffer)
{
	if(buffer == NULL)
		return 0;
	
	int len = size * nmemb;
	buffer->append(data, len);
	
	return len;
}

bool zHttpClient::httpPostNoReturn(std::string &ret, const std::string &postdata)
{
	ret.clear();
	char errorBuffer[CURL_ERROR_SIZE];
	reset();

	if(m_curl == NULL)
	{
		Fir::logger->error("curl_easy_init faild.");
		return false;
	}
	
	//设置本次操作为POST
	curl_easy_setopt(m_curl, CURLOPT_POST, 1);
	
	curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, zHttpClient::writer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ret);
	if(postdata.length() != 0)
	{
		curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, postdata.c_str());
	}
	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(m_curl);
	if(res != CURLE_OK)
	{
		Fir::logger->error("请求%s 失败，%d，%s", url.c_str(), res, errorBuffer);
		return false;
	}

	return true;
}

bool zHttpClient::httpGetNoReturn(std::string &ret, const std::string &getdata/*=""*/)
{
	ret.clear();
	char errorBuffer[CURL_ERROR_SIZE];
	reset();

	if (m_curl == NULL)
	{
		Fir::logger->error("m_curl is null.");
		return false;
	}

	//设置初始化
	curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);

	std::string geturl = url + "?";
	geturl += getdata;
	
	curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, zHttpClient::writer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ret);

	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(m_curl, CURLOPT_URL, geturl.c_str());
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(m_curl);
	if(res != CURLE_OK)
	{
		Fir::logger->error("请求%s 失败，%d，%s", geturl.c_str(), res, errorBuffer);
		return false;
	}

	return true;
}

void zHttpClient::reset()
{
	curl_easy_reset(m_curl);
}
