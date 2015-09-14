/*************************************************************************
 > File Name: plsa.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 15时41分23秒
 ************************************************************************/

#include<iostream>
#include<map>
#include<set>
#include<math.h>
#include<omp.h>
#include"util.h"
#include"const.h"
#include"tinyxml.h"
#include"tinystr.h"
#include"NLPIR.h"
#include<time.h>

#ifndef _PLSA_H_
#define _PLSA_H_

#define LM 18 //定义背景语言模型在数组中的标记

namespace nlp
{
	
	class Plsa
	{
		public:
			Plsa(std::string &conf_path);
			~Plsa();
			void train_plsa();
			bool get_topic_words(std::map<int, std::vector<std::string> > &topic_words);
			std::map<int, WeiboTopic_ICT::Weibo> get_documents() const; //文档id与文档对象映射关系

			void save_probs(std::string &path);
			bool load_probs(std::string &path);
		
		private:
			bool _load_config(std::string &path);
			bool _load_stop_words(std::set<std::string> &stop_set);
			bool _words_segement(std::map<int, std::string> &id_rawtext_map, 
					             std::map<std::string, int> &tmp_tf_map);
			bool _generate_dict(std::map<std::string, int> &tf_map, int min_thresh, std::set<std::string> &stop_set, std::map<std::string, int> &tmp_words_map);
			bool _text_preprocess(std::map<int, std::string> &id_rawtext_map, std::map<std::string, int> &tmp_words_map);
			bool _preprocess();
			void _init_probs();
			void _init_model();
			void _init_z_variable();
			void _calc_z_variable();
			void _calc_PI_variable();
			void _calc_term_prob();
			void _init_args();
			void _Estep();
			void _Mstep();
			void _destroy_intermedian_variable();


			tools::Util util;
			std::string _dict_path; //to rm
			std::string _doc_tf_path; // to rm
			std::string _local_path;
			std::string _stop_words_path;

			int _docs_nums;
			int _terms_nums;
			int _topics_nums;
			int _iter_nums;
			std::map<int, double> _doc_words_map; //统计每篇文档中的总词汇数
			std::map<int, WeiboTopic_ICT::Weibo> _id_doc_map; //文档id与文档对象映射关系
			
			double _lambda; 
			std::map<int, double> _tfs_map; //词频
			std::map<int, std::string> _literal_map; // id字面对应表
	        double **_doc_probs; //[MAX_DOCS][MAX_TOPICS]; //文档属于某个主题的概率 PI
	        double **_term_probs; //[MAX_TERMS][MAX_TOPICS]; //词属于某个主题的概率 P(w|j)
	        double ***_doc_term_probs; //[MAX_DOCS][MAX_TERMS][MAX_TOPICS]; //文档中的某个词属于某个主题的概率 Z(d,w)
	        double **_counts; //[MAX_DOCS][MAX_TERMS]; //某个词在某篇文档中出现的个数 c(d,w)

	};
}


#endif
