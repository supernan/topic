/*************************************************************************
 > File Name: util.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 10时11分19秒
 ************************************************************************/

#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<queue>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include"const.h"
#include"DataType.h"
#include"glog/logging.h"
#ifndef _UTIL_H_
#define _UTIL_H_

namespace tools
{
	class Trie
	{
		public:
			int count;
			int pattern_id;
			std::map<char, Trie*> next;
			Trie *fail;
			Trie()
			{
				next.clear();
				count = 0;
				pattern_id = -1;
				fail = NULL;
			}
	};

	class AC_automation
	{
		public:
			AC_automation();
			~AC_automation();
			std::map<int, std::string> query(std::string &text);
			void build_automation(std::vector<std::string> &patterns);
			void clear();
		private:
			void _build(Trie *root);
			void _insert(Trie *root, std::string &pattern, int id);
			void _destroy_tree(Trie *root);
			Trie *_root;
			std::map<int, int> _visit;
			std::map<int, std::string> _id_pattern_map;
	};

	class RelationTree
	{
		public:
			int find_parent(int pos, std::vector<int> &parents);
			void union_node(int x, int y, std::vector<int> &parents);
	};

	class Util
	{
		public:
			int docs_num;
			int terms_num;
			bool split_line(std::string &line, std::string &key, std::vector<std::string> &words_list);
			bool read_text_from_local(std::string &local, std::map<int, WeiboTopic_ICT::Weibo> &id_doc_map);
			void read_words_dict(std::string &dict_path, std::map<int, double> &dict_tf);
			void read_doc_tf(std::string &doc_path, double **counts,
					         std::map<int, double> &doc_words_map, std::map<int, double> &dict_tf);
	};


}

#endif

