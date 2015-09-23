/*************************************************************************
 > File Name: util.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 10时11分19秒
 ************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<queue>
#include"glog/logging.h"
#include"const.h"
#include"DataType.h"
#ifndef TOPIC_UTIL_H_
#define TOPIC_UTIL_H_

namespace tools
{
	/*字典树节点
	 *作为AC自动机参数
	 */
	struct Trie
	{
		int count; //标记是否为词结束节点
		int pattern_id; //如果该节点是模式的结束，记录匹配到的模式id
		std::map<char, Trie*> next; //下层节点
		Trie *fail;
		Trie()
		{
			next.clear();
			count = 0;
			pattern_id = -1;
			fail = NULL;
		}
	};

	
	/*AC自动机
	 *应用于多模式匹配
	 */
	class AC_automation
	{
		public:

			/*构造函数
			 *初始化字典树根节点
			 */
			AC_automation();


			/*析构函数
			 *释放字典树资源
			 */
			~AC_automation();


			/*文档查询
			 *在构建好的自动机基础上查询文本
			 *arg1:text[string] 要查询的文本
			 *ret[map] 返回匹配到的模式id与模式串的映射
			 */
			std::map<int, std::string> query(std::string &text);


			/*构造自动机
			 *插入所有模式串构造自动机
			 *arg1:patterns[vector] 要插入的所有模式
			 */
			void build_automation(std::vector<std::string> &patterns);
			
			
			/*清除自动机
			 *清除所有已经存在的模式串
			 */
			void clear();
		private:

			/*构造自动机
			 *配置节点的失效函数
			 *arg1:root[Trie] 自动机根节点
			 */
			void _build(Trie *root);


			/*插入模式
			 *向自动机中插入模式串
			 *arg1:root[Trie] 自动机的根节点
			 *arg2:pattern[string] 要插入的模式
			 *arg3:id[int] 模式串id
			 */
			void _insert(Trie *root, std::string &pattern, int id);


			/*释放自动机内存
			 *递归释放自动机Trie树的内存
			 *arg1:root[Trie] 根节点
			 */
			void _destroy_tree(Trie *root);
			
			Trie *_root; //Trie树根节点
			std::map<int, int> _visit; //在查询是标记模式串是否已经被匹配
			std::map<int, std::string> _id_pattern_map; //模式id与模式串的映射
	};

	
	/*并查集接口
	 *构建词与词关系的接口
	 */
	class RelationTreeInterface
	{
		public:

			/*并查集find
			 *查找节点的父亲节点
			 *arg1:pos[int] 当前节点
			 *arg2:parents[vector] 父亲节点数组
			 *ret[int] 父亲节点id
			 */
			static int find_parent(int pos, std::vector<int> &parents);

			
			/*并查集union
			 *将两个词进行union操作
			 *arg1:x[int] 节点x
			 *arg2:y[int] 节点y
			 *arg3:parents[vector] 父亲节点数组
			 */
			static void union_node(int x, int y, std::vector<int> &parents);
	};

	
	/*工具类接口
	 *封装了常用的功能函数
	 */
	class UtilInterface
	{
		public:

			/*字符串分割
			 *将字符串按照key分割
			 *arg1:line[string] 要分割的字符串
			 *arg2:key[string] 分隔符
			 *arg3:word_list[vector] 存储分割结果的数组
			 *ret[bool] 分割成功返回true否则返回false
			 */
			static bool split_line(std::string &line, std::string &key, std::vector<std::string> &words_list);
			
			
			/*从本地读取文件
			 *从本地读取文本数据并存入指定的数据结构
			 *arg1:local[string] 本地存储路径
			 *arg2:id_doc_map[map] 存储读取到的文档
			 *ret[bool] 读取成功返回true否则返回false
			 */
			static bool read_text_from_local(std::string &local, std::map<int, WeiboTopic_ICT::Weibo> &id_doc_map);
	};


}

#endif

