/**
 * \file HttpClient.h
 * \version  $Id$
 * \author  ,
 * \date 2013年07月18日 14时04分38秒 CST
 * \brief 定义http客户端服务类
 *
 * 
 */

#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <string>
#include "Fir.h"
#include "zNoncopyable.h"
#include <curl/curl.h>

class zHttpClient: public zNoncopyable
{
	public:
		/**
		 * \brief  构造函数
		 *
		 * \param requrl http请求
		 */
		zHttpClient(const std::string &requrl);
		zHttpClient();

		/**
		 * \brief  析构函数
		 *
		 */
		virtual ~zHttpClient();

		/**
		 * \brief  初始化curl
		 *
		 */
		int init_curl();
		
		/**
		 * \brief  初始化requrl
		 *
		 */
		void init_requrl(const std::string& requrl);


		/**
		 * \brief  post请求
		 *
		 *
		 * \param ret 保存请求的应答内容
		 * \param postdata post方式传数据
		 * \return 执行是否成功
		 */
		bool httpPost(std::string &ret, const std::string &postdata = "");
		/**
		 * \brief  get请求
		 *
		 *
		 * \param ret 保存请求的应答内容
		 * \param postdata post方式传数据
		 * \return 执行是否成功
		 */
		bool httpGet(std::string &ret, const std::string &postdata = "");

		/**
		 * \brief  没有返回值的post请求
		 *
		 *
		 * \param ret 保存请求的应答内容
		 * \param postdata post方式传数据
		 * \return 执行是否成功
		 */
		bool httpPostNoReturn(std::string &ret, const std::string &postdata = "");

		/**
		 * \brief  get请求
		 *
		 *
		 * \param ret 保存请求的应答内容
		 * \param getdata get方式传数据
		 * \return 执行是否成功
		 */
		bool httpGetNoReturn(std::string &ret, const std::string &getdata = "");


		DWORD state;
	private:
		/**
		 * \brief curl的回调函数，用于将服务器返回的字符串写到相应的string对象中
		 *
		 * \param data 待写入的数据
		 * \param size 数据元大小
		 * \param nmemb 数据元个数
		 * \param buffer 保存数据的缓存
		 * \return 执行是否成功
		 */
		static size_t writer(char *data, size_t size, size_t nmemb, std::string *buffer);
		void reset();

		std::string url;
		CURL* m_curl;
};

#endif
