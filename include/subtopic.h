/*************************************************************************
 > File Name: include/topic.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年09月14日 星期一 10时17分11秒
 ************************************************************************/
#include"plsa.h"
#include"util.h"
#ifndef TOPIC_TOPIC_H_
#define TOPIC_TOPIC_H_

namespace ict
{
	/*Topic 类封装了子话题发现的相关操作
	 *功能1:产生子话题及对应的关键词
	 *功能2:将不同文档归类到子话题
	 */
	class SubTopic
	{
		public:
			/*构造函数
			 * 初始化plsa对象,初始化自动机对象
			 *arg1: plsa对象所需的配置文件路径
			 */
			SubTopic(std::string &plsa_conf_path);

			
			/*析构函数
			 *释放plsa对象资源和自动机对象的资源
			 */
			~SubTopic();

			
			/*生成子话题和对应文档
			 *根据传入的文档生成子话题
			 *arg1:doc_list[vector] 要处理的语料文本
			 *arg2:topic_list[vector] 存储生成的所有子话题
			 *arg3:thresh[int] 两个词在句子中共现的最小阈值，此值越大对词对共现要求越严格，可能减少子话题关键词数
			 *arg4:min_count[int] 子话题最少关键词数，如果子话题关键词数少于此阈值，该子话题将被丢弃
			 *arg5:match_rate[double] 关键词匹配率0到1之间，此值越大对文本匹配度要求越高，将减少属于每个子话题的文本数
			 *ret[bool] 生成成功返回true否则返回false
			 */
			bool generate_sub_topics(std::vector<WeiboTopic_ICT::Weibo> &doc_list,
					                 std::vector<WeiboTopic_ICT::Topic> &topic_list,
									 int thresh=1, int min_count=3, double match_rate=0.2);

		private:
			
			/*产生子话题
			 *产生语料对应的子话题
			 *arg1:thresh[int] 两个词语在句子中共现的最小阈值
			 *arg2:min_count[int] 子话题最少关键词数
			 *ret[bool]: 如果成功生成子话题返回true否则false
			 */
			bool _generate_topic_words(int thresh, int min_count);


			/*判断文档所属子话题
			 *根据产生的子话题关键词匹配度，决定文档属于的子话题
			 *arg1:match_rate[double] 文档关键词的匹配度取值０到１
			 *ret[bool]:如果函数成功执行返回true否则返回false
			 */
			bool _generate_docs(double match_rate);
			
			
			/*产生PLSA子话题
			 *调用plsa对象执行训练，获得子话题和关键词
			 */
			void _get_plsa_topics();


			/*统计PLSA子话题关键词在句子中的共现情况
			 *根据关键词的匹配情况填充共现矩阵
			 *arg1:key_words[vector<string>] 子话题对用的关键词
			 *arg2:text[string] 判断的文本
			 *arg3:matrix[vector<vector<int>>] 要填充的共现矩阵
			 */
			void _words_occurrence(std::vector<std::string> &key_words, std::string &text,
					               std::vector<std::vector<int> > &matrix);


			/*判断词语之间的关联关系
			 *根据共现矩阵利用并查集判断词语之间的关系，对PLSA每个子话题的关键词进行聚类
			 *arg1:matrix[vector<vector<int>>] 词语共现矩阵
			 *arg2:thresh[int] 共现次数最小阈值，判断两个词是否相关
			 *arg3:key_words[vector<string>] 每个子话题的关键词
			 *arg4:words_cluster[map<int, vector>] 存储聚类后的子话题关键词
			 */
			void _words_relations(std::vector<std::vector<int> > &matrix, int thresh, 
								  std::vector<std::string> &key_words,
								  std::map<int, std::vector<std::string> > &words_cluster);
			
			
			tools::AC_automation *_mation_ptr; //自动机对象指针
			nlp::Plsa *_plsa_ptr; //plsa对象指针

			std::map<int, std::vector<std::string> > _topic_words; //生成的子话题结果
			std::map<int, std::vector<WeiboTopic_ICT::Weibo> > _topic_docs; //文档按子话题聚类的结果
	};
}


#endif 
