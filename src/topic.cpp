/*************************************************************************
 > File Name: src/topic.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年09月14日 星期一 10时28分37秒
 ************************************************************************/

#include"topic.h"

ict::Topic::Topic(std::string &plsa_conf_path)
{
	_plsa_ptr = new nlp::Plsa(plsa_conf_path);
	_mation_ptr = new tools::AC_automation();
}


ict::Topic::~Topic()
{
	delete _plsa_ptr;
	delete _mation_ptr;
}


void ict::Topic::_get_plsa_topics()
{
	LOG(INFO) << "get plsa topic words" <<std::endl;
	_plsa_ptr->train_plsa();
	LOG(INFO) << "plsa terminate" << std::endl;
}


void ict::Topic::_words_occurrence(std::vector<std::string> &key_words, std::string &text,
		                           std::vector<std::vector<int> > &matrix)
{
	std::vector<std::string> sents;
	std::string end_sep = "。";
	tools::UtilInterface::split_line(text, end_sep, sents); //断句计算词语共现
	for (unsigned int i = 0; i < sents.size(); ++i)
	{
		std::map<int, std::string> match_patterns = _mation_ptr->query(sents[i]);
		std::map<int, std::string>::iterator iter;
		std::vector<int> match_ids;
		/*if (match_patterns.size() > 0)
		{
			std::cout<<"text:"<<text<<std::endl;
			std::cout<<"keys: "<<std::endl;
			for (int i = 0; i < key_words.size(); i++)
				std::cout<<key_words[i]<<" ";
			std::cout<<std::endl;
		}*/
		for (iter = match_patterns.begin(); iter != match_patterns.end(); ++iter)
		{
			match_ids.push_back(iter->first);
		}
		//if (match_ids.size() > 0)
		//	std::cout<<"end"<<std::endl;
		for (unsigned int i= 0; i < match_ids.size(); ++i)
		{
			for (unsigned int j = 0; j < match_ids.size(); ++j)
			{
				if (match_ids[i] != match_ids[j])
					matrix[match_ids[i]][match_ids[j]] += 1;
			}
		}

	}
}


void ict::Topic::_words_relations(std::vector<std::vector<int> > &matrix, int thresh,
		                         std::vector<std::string> &key_words,
								 std::map<int, std::vector<std::string> > &words_cluster)
{
	int m = matrix.size();
	int n = matrix[0].size();
	std::vector<int> parents(m, 0);
	for (int i = 0; i < m; ++i)
		parents[i] = i;
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			if (matrix[i][j] >= thresh)
			{
				tools::RelationTreeInterface::union_node(i, j, parents);
			}
		}
	}
	
	for (int i = 0; i < m; ++i)
	{
		int id = parents[i];
		std::vector<std::string> words;
		words_cluster[id] = words;
	}
	for (int i = 0; i < m; ++i)
	{
		words_cluster[parents[i]].push_back(key_words[i]);
	}
}


bool ict::Topic::generate_topics(int thresh, int min_count)
{
	LOG(INFO) << "try to get topic words" << std::endl;
	_get_plsa_topics();
	std::map<int, std::vector<std::string> > plsa_topic_words;
	if (!_plsa_ptr->get_topic_words(plsa_topic_words))
	{
		LOG(ERROR) << "get plsa topic words failed" << std::endl;
		return false;
	}
	std::map<int, WeiboTopic_ICT::Weibo> docs = _plsa_ptr->get_documents();
	std::map<int, std::vector<std::string> >::iterator iter;
	std::map<int, WeiboTopic_ICT::Weibo>::iterator doc_iter;
	if (docs.empty())
	{
		LOG(ERROR) << "there is no any doc" <<std::endl;
		return false;
	}
	int topic_count = 0;
	_mation_ptr->clear();
	for (iter = plsa_topic_words.begin(); iter != plsa_topic_words.end(); ++iter)
	{
		std::vector<std::string> key_words = iter->second;
		_mation_ptr->build_automation(key_words); //构建自动机
		int len = key_words.size();
		std::vector<int> data(len, 0);
		std::vector<std::vector<int> > matrix(len, data); //共现矩阵
		for (doc_iter = docs.begin(); doc_iter != docs.end(); ++doc_iter)
		{
			std::string text = doc_iter->second.mt;
			_words_occurrence(key_words, text, matrix);
		}
		/*for (int i = 0; i < matrix.size(); i++)
		{
			for (int j = 0; j < matrix[0].size(); j++)
				std::cout<<matrix[i][j]<<" ";
			std::cout<<std::endl;
		}*/
		_mation_ptr->clear(); //清除自动机
		std::map<int, vector<std::string> > words_cluster;
		_words_relations(matrix, thresh, key_words, words_cluster);
		
		std::map<int, vector<std::string> >::iterator word_iter;
		for (word_iter = words_cluster.begin(); word_iter != words_cluster.end(); ++word_iter)
		{
			if (word_iter->second.size() >= static_cast<unsigned int>(min_count))
			{
				_topic_words[topic_count] = word_iter->second;
				for (unsigned int i = 0; i < _topic_words[topic_count].size(); i++)
					std::cout<<_topic_words[topic_count][i]<<" ";
				std::cout<<std::endl;
				topic_count++;
			}
		}

	}
	LOG(INFO) << "get topic words succeed" << std::endl;
	return true;
}


bool ict::Topic::generate_docs(double match_rate)
{
	LOG(INFO) << "try to generate docs of each topics" << std::endl;
	if (_topic_words.empty())
	{
		LOG(ERROR) << "topic words can not be empty！" << std::endl;
		return false;
	}
	std::map<int, std::vector<std::string> >::iterator topic_iter;
	std::map<int, std::vector<WeiboTopic_ICT::Weibo> >::iterator weibo_iter;
	std::map<int, WeiboTopic_ICT::Weibo>::iterator doc_iter;
	std::map<int, WeiboTopic_ICT::Weibo> docs = _plsa_ptr->get_documents();
	std::cout<<"topic size: "<<_topic_words.size()<<std::endl;
	_mation_ptr->clear();
	for (topic_iter = _topic_words.begin(); topic_iter != _topic_words.end(); ++topic_iter)
	{
		std::vector<std::string> key_words = topic_iter->second;
		_mation_ptr->build_automation(key_words);
		int key_size = key_words.size(); 
		std::vector<WeiboTopic_ICT::Weibo> texts;
		_topic_docs[topic_iter->first] = texts;
		for (doc_iter = docs.begin(); doc_iter != docs.end(); ++doc_iter)
		{
			std::map<int, std::string> match_patterns = _mation_ptr->query(doc_iter->second.mt);
			int match_size = match_patterns.size();
			if (match_size > key_size * match_rate)
			{
				_topic_docs[topic_iter->first].push_back(doc_iter->second);
			}
		}
		_mation_ptr->clear();
	}
	for (weibo_iter = _topic_docs.begin(); weibo_iter != _topic_docs.end(); ++weibo_iter)
	{
		std::cout <<weibo_iter->first<<" "<<"size: "<<weibo_iter->second.size()<<std::endl;
	}
	LOG(INFO) << "generate docs of each topics succeed" << std::endl;
	return true;
}




