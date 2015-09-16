/*************************************************************************
 > File Name: include/topic.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年09月14日 星期一 10时17分11秒
 ************************************************************************/
#include"plsa.h"
#include"util.h"
#ifndef _TOPIC_H_
#define _TOPIC_H_

namespace ict
{
	class Topic
	{
		public:
			Topic(std::string &plsa_conf_path);
			~Topic();
			bool generate_topics(int thresh, int min_count);
			bool generate_docs(double match_rate);
			

		private:
			void _get_plsa_topics();
			void _words_occurrence(std::vector<std::string> &key_words, std::string &text,
					               std::vector<std::vector<int> > &matrix);
			void _words_relations(std::vector<std::vector<int> > &matrix, int thresh, 
								  std::vector<std::string> &key_words,
								  std::map<int, std::vector<std::string> > &words_cluster);
			
			tools::Util *_util_ptr;
			tools::AC_automation *_mation_ptr;
			nlp::Plsa *_plsa_ptr;

			std::map<int, std::vector<std::string> > _topic_words;
			std::map<int, std::vector<WeiboTopic_ICT::Weibo> > _topic_docs;
	};
}


#endif 
