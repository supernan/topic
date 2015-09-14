/*************************************************************************
 > File Name: test.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: Thu 10 Sep 2015 02:51:16 PM CST
 ************************************************************************/

#include<iostream>
#include"const.h"
#include"util.h"
#include"plsa.h"
#include"topic.h"
using namespace std;
int main(int argc, char **argv)
{
	google::InitGoogleLogging(argv[0]);
	google::SetStderrLogging(google::ERROR);
	google::SetLogDestination(google::GLOG_INFO,"../log/");
	FLAGS_colorlogtostderr=true;
	string conf_path = "../conf/plsa_conf.xml";
	ict::Topic topic(conf_path);
	topic.generate_topics(1, 2);
	//string prob_path = "../data";
	//nlp::Plsa plsa(conf_path);
	//plsa.load_probs(prob_path);
	//plsa.train_plsa();
	//map<int, vector<string> > v;
	//plsa.get_topic_words(v);
	
	/*()tools::AC_automation ac;
	string word1 = "我爱";
	string word2 = "asdasdas阿富汗";
	string word3 = "国人";
	vector<string> w;
	w.push_back(word1);
	w.push_back(word2);
	w.push_back(word3);

	string text = "我是中国人, 我爱中国我爱";
	ac.build_automation(w);
	ac.clear();
	ac.build_automation(w);
	map<int, string> ret = ac.query(text);
	cout<<ret.size()<<endl;
	map<int, string>::iterator it;
	for (it = ret.begin(); it != ret.end(); ++it)
		cout<<it->first<<" "<<it->second<<endl;*/
	
	/*string path = "../../tianjin_text";
	vector<WeiboTopic_ICT::Weibo> doc_list;
	tools::Util util;
	cout<<util.read_text_from_local(path, doc_list);*/

	return 0;
}

