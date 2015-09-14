#ifndef __DATATYPE_H__
#define __DATATYPE_H__

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
using namespace std;

namespace WeiboTopic_ICT
{
	typedef pair<int, double> feature;		//svm feature: index:value

	/*!
	* \class	_stWord
	* \brief	word structure
	* \author	chenqiang(chenqiang@software.ict.ac.cn) 
	* \date		2013-12-09
	*/
	typedef struct __stWord          
	{
		string  m_sWord;        //word content
		double m_fWeight;       //word weight
		double m_fTf;           //word tf
		string pos;				//word pos
		__stWord()
		{                //constructor
			m_sWord = ""; 
			m_fWeight = 0.0;
			m_fTf = 0.0;
			pos = "";
		}   
	} Word;

	/*!
	* \class	_stWeibo
	* \brief	weibo structure
	* \author	chenqiang(chenqiang@software.ict.ac.cn) 
	* \date		2013-12-09
	*/
	typedef struct __stWeibo
	{
		int index;					//reserved keywords
		string id;					//text id
		string source;				//original text
		string ui;					//user id
		string mt;					//messsage text
		int cc;						//comment count
		int rc;						//retweet count
		string nc;					//name card	
		string rpi;					//retweet_parent_msg_id
		string ri;					//retweet_msg_id
		string ru;					//转发用户ID(ri用户消息的作者)
		vector<Word> keywords;		//keywords
		string keyvect;				//keyvect
		string pos;					//pos info
		vector<string> ol;			//organization list
		vector<string> pl;			//person list
		vector<string> rl;			//region list
		string pic;					//pictures
		string video;				//video
		string audio;				//audio	
		long long ct;				//crawl time
		long long it;				//insert time
		long long pt;				//publish time
		int ir;						//is retweet
		int ed;						//evidence_degree
		int imd;					//importance_degree
		vector<double> latentDim;	//the doc latentDim using svd or other methods
	}stWeibo, Weibo, *pstWeibo;

	/*!
	* \class	_stUser
	* \brief	weibo user structure
	* \author	chenqiang(chenqiang@software.ict.ac.cn) 
	* \date		2013-12-09
	*/
	typedef struct __stUser
	{
		string id;		//user id
		string un;		//user name
		string sn;		//screen name
		string sx;		//sex
		string ad;		//address
		string de;		//description
		int an;			//followers num
		int fn;			//friends num
		int mn;			//message num
		int iv;			//is_verified
		string vi;		//verify_info
		int at;			//create_time
	}stUser, User, *pstUser;

	typedef struct __stTopic
	{
		string title;					//topic title
		string keyvect;					//(word)[TF];
		string keywords;				//word1;word2;...wordn;
		int totalCC;					//total cc
		int totalRC;					//total rc
		int totalWeiboNum;				//total weibo num before duplicate removal
		double hotDegree;				//hot degree
		vector<Weibo> weibovec;	
	}Topic, *_pstTopic;

}

#endif
