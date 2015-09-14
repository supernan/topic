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
	_util_ptr = new tools::Util();
	_mation_ptr = new tools::AC_automation();
}


ict::Topic::~Topic()
{
	delete _plsa_ptr;
	delete _util_ptr;
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
	std::map<int, std::string> match_patterns = _mation_ptr->query(text);
	std::map<int, std::string>::iterator iter;
	std::vector<int> match_ids;
	for (iter = match_patterns.begin(); iter != match_patterns.end(); ++iter)
		match_ids.push_back(iter->first);
	for (int i= 0; i < match_ids.size(); i++)
	{
		for (int j = 0; j < match_ids.size(); j++)
		{
			if (i != j)
				matrix[i][j] += 1;
		}
	}
}


void ict::Topic::_words_relations(std::vector<std::vector<int> > &matrix, int thresh,
		                         std::vector<std::string> &key_words,
								 std::map<int, std::vector<std::string> > &words_cluster)
{
	tools::RelationTree relations;
	int m = matrix.size();
	int n = matrix[0].size();
	std::vector<int> parents(m, 0);
	for (int i = 0; i < m; i++)
		parents[i] = i;
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (matrix[i][j] >= thresh)
			{
				relations.union_node(i, j, parents);
			}
		}
	}
	
	for (int i = 0; i < m; i++)
	{
		int id = parents[i];
		std::vector<std::string> words;
		words_cluster[id] = words;
	}
	for (int i = 0; i < m; i++)
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
	for (iter = plsa_topic_words.begin(); iter != plsa_topic_words.end(); ++iter)
	{
		std::vector<std::string> key_words = iter->second;
		_mation_ptr->build_automation(key_words); //构建自动机
		int len = key_words.size();
		std::vector<int> data(len, 0);
		vector<vector<int> > matrix(len, data); //共现矩阵
		for (doc_iter = docs.begin(); doc_iter != docs.end(); ++doc_iter)
		{
			std::string text = doc_iter->second.mt;
			_words_occurrence(key_words, text, matrix);
		}
		_mation_ptr->clear(); //清除自动机
		std::map<int, vector<std::string> > words_cluster;
		_words_relations(matrix, thresh, key_words, words_cluster);
		
		int topic_count = 0;
		std::map<int, vector<std::string> >::iterator word_iter;
		for (word_iter = words_cluster.begin(); word_iter != words_cluster.end(); ++word_iter)
		{
			if (word_iter->second.size() > min_count)
			{
				_topic_words[topic_count] = word_iter->second;
				for (int i = 0; i < _topic_words[topic_count].size(); i++)
					std::cout<<_topic_words[topic_count][i]<<" ";
				std::cout<<std::endl;
				topic_count += 1;
			}
		}

	}
	LOG(INFO) << "get topic words succeed" << std::endl;
	return true;
}






