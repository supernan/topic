/*************************************************************************
 > File Name: plsa.h
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 15时41分23秒
 ************************************************************************/

#include<time.h>
#include<omp.h>
#include<math.h>
#include<memory.h>
#include<iostream>
#include<map>
#include<set>
#include"tinyxml.h"
#include"tinystr.h"
#include"NLPIR.h"
#include"util.h"
#include"const.h"

#ifndef TOPIC_PLSA_H_
#define TOPIC_PLSA_H_

#define LM 18 //定义背景语言模型在数组中的标记

namespace nlp
{
	/*封装PLSA with Background Language全部操作
	 *功能1:训练模型
	 *功能2:产生关键词
	 *功能3:保存模型参数
	 *功能4:加载模型参数初始化对象
	 */
	class Plsa
	{
		public:

			/*构造函数
			 *加载配置文件初始化类参数
			 *arg1:conf_path[string] 配置文件路径
			 */
			Plsa(std::string &conf_path);


			/*析构函数
			 *释放模型参数所占据的内存
			 */
			~Plsa();


			/*训练PLSA模型
			 *预处理文本，训练模型，保存参数并在训练成功后释放中间变量的内存
			 *ret[bool] 如训练成功返回true遇到异常返回false
			 */
			bool train_plsa();


			/*获得PLSA产生的子话题关键词
			 *根据每个子话题的词频排序返回top10的关键词
			 *arg1:topic_words[map<int,vector>] 存储每个子话题关键词的map
			 *ret[bool] 成功获取关键词返回true否则返回false
			 */
			bool get_topic_words(std::map<int, std::vector<std::string> > &topic_words);
			
			
			/*返回文档语料
			 *返回预处理成功的语料
			 *ret[map<int, doc>] 
			 */
			std::map<int, WeiboTopic_ICT::Weibo> get_documents() const; //文档id与文档对象映射关系

			
			/*保存训练好的模型参数
			 *将训练好的doc-topic word-topic概率保存
			 *arg1:path[string] 保存路径
			 */
			void save_probs(std::string &path);
			
			
			/*加载训练好的参数
			 *arg1:path[string] 模型参数路径
			 *ret[bool]: 加载成功返回true否则返回false
			 */
			bool load_probs(std::string &path);


			/*设置语料集合
			 *接受从外部传入的语料
			 */
			void set_documents(std::vector<WeiboTopic_ICT::Weibo> &doc_list);
		
		private:

			/*加载配置文件
			 *根据路径加载配置文件初始化模型参数
			 *arg1:path[string] 配置文件路径
			 *ret[bool]: 成功加载返回true否则返回false
			 */
			bool _load_config(std::string &path);


			/*加载停用词表
			 *从外部加载停用词
			 *arg1:stop_set[set] 存储停用词的集合
			 *ret[bool] 成功加载返回true否则返回false
			 */
			bool _load_stop_words(std::set<std::string> &stop_set);


			/*分词
			 *读取文本分词，为生成词典做准备工作
			 *arg1:id_rawtext_map[map] 存储分词后的文本以空格隔开
			 *arg2:tmp_tf_map[map] 存储初步的词频统计
			 *ret[bool] 分词成功返回true否则返回false
			 */
			bool _words_segement(std::map<int, std::string> &id_rawtext_map, 
					             std::map<std::string, int> &tmp_tf_map);


			/*生成词典
			 *根据初步的分词结果去除停用词，选择词频大于阈值的词进入词典
			 *arg1:tf_map[map] 临时词频统计
			 *arg2:min_thresh[int] 最小词频阈值
			 *arg3:stop_set[set] 停用词集合
			 *arg4:tmp_words_map[map] 词字面与id的映射关系
			 *ret[bool] 词典生成成功返回true否则返回false
			 */
			bool _generate_dict(std::map<std::string, int> &tf_map, int min_thresh, std::set<std::string> &stop_set, std::map<std::string, int> &tmp_words_map);
			
			
			/*文本预处理
			 *对分词后的文本统计每篇文档的tf
			 *arg1:id_rawtext_map[map] 分词结束的文本
			 *arg2:tmp_words_map[map] 词语字面与id的映射
			 *ret[bool] 预处理成功返回true否则返回false
			 */
			bool _text_preprocess(std::map<int, std::string> &id_rawtext_map, std::map<std::string, int> &tmp_words_map);
			
			
			/*预处理
			 *包括读取文件，分词生成词典，文本预处理
			 *ret[bool] 预处理成功返回true否则返回false
			 */
			bool _preprocess();


			/*初始化模型概率参数
			 *doc-topic word-topic 数组空间申请，并初始化
			 */
			void _init_probs();


			/*初始化EM算法中间隐变量
			 *为隐变量初始化并完成0初始化
			 */
			void _allocate_latent_variable();
			
			
			/*随机初始化隐变量
			 *为隐变量赋予随机值并归一化
			 *arg1:id[int] 文档id
			 */
			void _init_latent_variable(int id);


			/*迭代中计算隐变量
			 *EM算法计算隐变量
			 *arg1:id[int] 文档id
			 *arg2:term_probs[double**] 计算隐变量所需的词语话题分布
			 */
			void _calc_latent_variable(int id, double **term_probs);


			/*算法参数初始化
			 *EM算法所需参数的初始化
			 */
			void _init_EM();


			/*实现EM算法
			 *arg1:term_flg[int] 开关变量初始值为1
			 */
			void _EM_process(bool &term_flg);


			/*释放中间变量
			 *释放隐变量和doc-tf数组的内存空间
			 */
			void _destroy_intermedian_variable();

			
			/*计算单独一篇文档的话题分布
			 * arg1:id[int] 文档id
			 */
			void _calc_single_doc_prob(int id);


			/*计算单独一个词的话题分布
			 *arg1:id[int] 文档id
			 *arg2:total[double*] 各个话题的累计量
			 *arg3:term_prob[double**] 需要更新的词语话题分布变量
			 */
			void _calc_term_prob(int id, double *total, double **term_probs);


			/*清空临时term_prob对象
			 *arg1:term_prob[double**] 要重置的词语话题分布变量
			 */
			void _clear_term_probs(double **term_probs);


			/*词语话题分布归一化
			 *arg1:term_probs[double**] 需要归一化的词语话题分布
			 *arg2:total[double*] 针对不同话题统计的累计量
			 */
			void _term_normalization(double **_term_probs, double *total);


			/*将词语话题分布归并得到最终计算结果
			 */
			void _merge_term_probs();
			

			std::string _local_path; //本地语料路径
			std::string _stop_words_path; //停用词路径

			int _docs_nums; //文档数
			int _terms_nums; //词汇数
			int _topics_nums; //话题数
			int _iter_nums; //迭代次数
			std::map<int, double> _doc_words_map; //统计每篇文档中的总词汇数
			std::map<int, WeiboTopic_ICT::Weibo> _id_doc_map; //文档id与文档对象映射关系
			
			double _lambda; //背景语言模型所占比重参数
			std::map<int, double> _tfs_map; //词频
			std::map<int, std::string> _literal_map; // id字面对应表
	        double **_doc_probs; //[MAX_DOCS][MAX_TOPICS]; //文档属于某个主题的概率 PI
	        double **_term_probs; //[MAX_TERMS][MAX_TOPICS]; //词属于某个主题的概率 P(w|j)
			double **_term_probs_bak; //更新词语话题分布时的辅助变量
	        double **_latent; //优化处理所需的隐变量 [V][K]
	        double **_counts; //[MAX_DOCS][MAX_TERMS]; //某个词在某篇文档中出现的个数 c(d,w)
			//std::map<int, std::map<int, double> > _counts; //[MAX_DOCS][MAX_TERMS]; //某个词在某篇文档中出现的个数 c(d,w)

	};
}


#endif
