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
	topic.generate_topics(1, 3);
	topic.generate_docs(0.2);
	//string prob_path = "../data";
	//nlp::Plsa plsa(conf_path);
	//plsa.load_probs(prob_path);
	//plsa.train_plsa();
//map<int, vector<string> > v;
	//plsa.get_topic_words(v);
	
	/*tools::AC_automation ac;
	string word1 = "抢救";
	string word2 = "采访";
	string word3 = "生命体";
	vector<string> w;
	w.push_back(word1);
	w.push_back(word2);
	w.push_back(word3);

	string text = "视频：天津爆炸事故现场已搜救出32人 来源：中央电视台 记者在救援前方指挥部采访过程中，指挥员的步话机突然响起——火场里又抢救出一人，可惜已经没有生命体征“我们尽全力搜救，保留生的希望，即便是逝者，也要给他们最后的尊严。”";
	string text2 = "视频：天津爆炸事故现场已搜救出32人 来源：中央电视台 记者在救援前方指挥部采访过程中，指挥员的步话机突然响起——火场里又抢救出一人，可惜已经没有生命体征“我们尽全力搜救，保留生的希望，即便是逝者，也要给他们最后的尊严。”";
	ac.build_automation(w);
	ac.clear();
	ac.build_automation(w);
	map<int, string> ret = ac.query(text);
	map<int, string> ret2 = ac.query(text2);
	cout<<ret.size()<<endl;
	map<int, string>::iterator it;
	for (it = ret2.begin(); it != ret2.end(); ++it)
		cout<<it->first<<" "<<it->second<<endl;*/
	
	/*string path = "../../tianjin_text";
	vector<WeiboTopic_ICT::Weibo> doc_list;
	tools::Util util;
	cout<<util.read_text_from_local(path, doc_list);*/

	return 0;
}

