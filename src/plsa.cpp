/*************************************************************************
 > File Name: ../src/plsa.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 16时06分53秒
 ************************************************************************/

#include"plsa.h"

bool nlp::Plsa::_load_config(std::string &path)
{
	if ((access(path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Configuration file not found" << std::endl;
		return false;
	}

	TiXmlDocument *plsa_config = new TiXmlDocument(path.c_str());
	plsa_config->LoadFile(); 
	TiXmlElement *root_element = plsa_config->RootElement();
	TiXmlElement *topic_node = root_element->FirstChildElement();
	TiXmlElement *iter_node = topic_node->NextSiblingElement();
	TiXmlElement *lambda_node = iter_node->NextSiblingElement();
	TiXmlElement *local_path_node = lambda_node->NextSiblingElement();
	TiXmlElement *stop_path_node = local_path_node->NextSiblingElement();
	TiXmlElement *dict_node = stop_path_node->NextSiblingElement();
	TiXmlElement *doc_node = dict_node->NextSiblingElement();

	std::string topics_str = topic_node->FirstChild()->Value();
	std::string iters_str = iter_node->FirstChild()->Value();
	std::string lambda_str = lambda_node->FirstChild()->Value();
	std::string local_path = local_path_node->FirstChild()->Value();
	std::string stop_path = stop_path_node->FirstChild()->Value();
	std::string dict_path_str = dict_node->FirstChild()->Value();
	std::string doc_path_str = doc_node->FirstChild()->Value();

	_topics_nums = atoi(topics_str.c_str());
	_iter_nums = atoi(iters_str.c_str());
	_lambda = atof(lambda_str.c_str());
	_local_path = local_path;
	_stop_words_path = stop_path;

	delete plsa_config;
	LOG(INFO) << "Load configuration succeed"<<std::endl;
	return true;
}


bool nlp::Plsa::_load_stop_words(std::set<std::string> &stop_set)
{
	LOG(INFO) << "Load stop words" <<std::endl;
	if ((access(_stop_words_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Stop words file not found" << std::endl;
		return false;
	}
	std::ifstream input_stream(_stop_words_path.c_str());
	std::string stop_word;
	while (getline(input_stream, stop_word))
	{
		stop_set.insert(stop_word);
	}
	input_stream.close();
	LOG(INFO) << "Load stop words succeed" <<std::endl;
	return true;
}


bool nlp::Plsa::_words_segement(std::map<int, std::string> &index_rawtext_map, 
		                        std::map<std::string, int> &tmp_tf_map)
{
	LOG(INFO) << "Words segement" <<std::endl;
	if (!NLPIR_Init("../data/", UTF8_CODE))
	{
		LOG(FATAL) << "ICTCLAS init error" <<std::endl;
		return false;
	}
	std::string sep = " ";
	std::map<int, WeiboTopic_ICT::Weibo>::iterator iter;
	for (iter = _id_doc_map.begin(); iter != _id_doc_map.end(); ++iter)
	{
		WeiboTopic_ICT::Weibo doc = iter->second;
		std::string text = doc.mt;
		int id = doc.index;
		const char *result;
		result = NLPIR_ParagraphProcess(text.c_str(), 0);
		std::string raw_line = result;
		index_rawtext_map[id] = raw_line; //保留文档分词后的结果
		std::vector<std::string> words;
		if (!tools::UtilInterface::split_line(raw_line, sep, words))
		{
			LOG(ERROR) << "Error when cut words " <<std::endl;
			return false;
		}
		for (unsigned int i = 0; i < words.size(); ++i)
			tmp_tf_map[words[i]]++; //词频统计
	}
	NLPIR_Exit();
	_docs_nums = index_rawtext_map.size(); //获得文档总数
	LOG(INFO) << "Words segement succeed" <<std::endl;
	return true;
}


bool nlp::Plsa::_generate_dict(std::map<std::string, int> &tf_map, int min_thresh, std::set<std::string> &stop_set, 
		                       std::map<std::string, int> &tmp_words_map)
{
	LOG(INFO) << "Dict generate" <<std::endl;
	if (tf_map.empty())
	{
		LOG(ERROR) << "There is no word to generate dict" <<std::endl;
		return false;
	}
	std::map<std::string, int>::iterator iter;
	std::set<std::string>::iterator set_iter;
	int count = 0;
	int total = 0;
	for (iter = tf_map.begin(); iter != tf_map.end(); ++iter)
	{
		set_iter = stop_set.find(iter->first); //检查是否在停用词表中
		if (set_iter != stop_set.end())
			continue;
		if (iter->second > min_thresh)
		{
			_tfs_map[count] = iter->second;
			tmp_words_map[iter->first] = count;
			_literal_map[count] = iter->first;
			total += iter->second;
			count++;
		}
	}
	if (_tfs_map.empty() || total == 0)
	{
		LOG(ERROR) << "Every tf is too low" <<std::endl;
		return false;
	}
	std::map<int, double>::iterator dict_iter;
	for (dict_iter = _tfs_map.begin(); dict_iter != _tfs_map.end(); ++dict_iter)
	{
		dict_iter->second = double(dict_iter->second) / double(total);
	}
	LOG(INFO) << "Dict generate succeed" <<std::endl;
	_terms_nums = _tfs_map.size();
	return true;
}


bool nlp::Plsa::_text_preprocess(std::map<int, std::string> &id_rawtext_map,
		                         std::map<std::string, int> &tmp_words_map)
{
	LOG(INFO) << "Text preprocess" <<std::endl;
	if (id_rawtext_map.empty())
	{
		LOG(INFO) << "All text is empty" <<std::endl;
		return false;
	}

	_counts = new double*[_docs_nums];
	for (int i = 0; i < _docs_nums; ++i)
		_counts[i] = new double[_terms_nums];
	for (int i = 0; i < _docs_nums; ++i)
		for (int j = 0; j < _terms_nums; ++j)
			_counts[i][j] = 0;

	std::map<int, std::string>::iterator iter;
	std::map<int, double>::iterator dict_iter;
	std::string sep = " ";
	for (iter = id_rawtext_map.begin(); iter != id_rawtext_map.end(); ++iter)
	{
		int id = iter->first;
		std::string raw_text = iter->second;
		std::vector<std::string> words;
		if (!tools::UtilInterface::split_line(raw_text, sep, words))
		{
			LOG(ERROR) << "Text error when split raw_text" <<std::endl;
			return false;
		}
		for (unsigned int i = 0; i < words.size(); ++i)
		{
			std::map<std::string, int>::iterator find_iter;
			find_iter = tmp_words_map.find(words[i]);
			if (find_iter == tmp_words_map.end())
				continue;
			int word_id = tmp_words_map[words[i]]; //
			dict_iter = _tfs_map.find(word_id);
			if (dict_iter == _tfs_map.end())
				continue;
			_counts[id][word_id] += 1;
			_doc_words_map[id] += 1;
		}
	}
	LOG(INFO) << "Text preprocess succeed" <<std::endl;
	return true;
}


bool nlp::Plsa::_preprocess()
{
	LOG(INFO) << "Preprocess" <<std::endl;
	std::map<int, std::string> index_rawtext_map; //临时变量, 存储文档id到分词后文本的映射
	std::map<std::string, int> tmp_tf_map; //临时变量,初步分词结果到词频的映射
	std::map<std::string, int> tmp_word_map; //临时变量, 字典中词到词id的映射
	std::set<std::string> stop_set;
	if (_id_doc_map.empty())
	{
		std::vector<WeiboTopic_ICT::Weibo> doc_list;
		if (!tools::UtilInterface::read_text_from_local(_local_path, doc_list))
		{
			LOG(ERROR) << "Read text from local failed" <<std::endl;
			return false;
		}
		set_documents(doc_list);
	}
	if (!_load_stop_words(stop_set))
	{
		LOG(ERROR) << "Load stop words failed" <<std::endl;
		return false;
	}
	if (_id_doc_map.empty())
	{
		LOG(ERROR) << "Doc list is empty" <<std::endl;
		return false;
	}
	if (!_words_segement(index_rawtext_map, tmp_tf_map))
	{
		LOG(FATAL) << "Words segement failed" << std::endl;
		return false;
	}
	if (!_generate_dict(tmp_tf_map, 1, stop_set, tmp_word_map))
	{
		LOG(FATAL) << "Generate dict failed"<<std::endl;
		return false;
	}
	if (!_text_preprocess(index_rawtext_map, tmp_word_map))
	{
		LOG(FATAL) << "Text preprocess failed" <<std::endl;
		return false;
	}
	LOG(INFO) << "Preprocess succeed" <<std::endl;
	return true;
}


void nlp::Plsa::_init_probs()
{
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			_doc_probs = new double*[_docs_nums];
			{
				for (int i = 0; i < _docs_nums; ++i)
				{
					_doc_probs[i] = new double[MAX_TOPICS];
				}
			}
		}

		#pragma omp section
		{
			_term_probs = new double*[_terms_nums];
			for (int j = 0; j < _terms_nums; ++j)
			{
				_term_probs[j] = new double[MAX_TOPICS];
			}
		}

	}
	
	for (int i = 0; i < _docs_nums; ++i)
	{
		for (int k = 0; k < MAX_TOPICS; ++k)
		{
			_doc_probs[i][k] = 0.0;
		}
	}

	for (int i = 0; i < _terms_nums; ++i)
	{
		for (int k = 0; k < MAX_TOPICS; ++k)
			_term_probs[i][k] = 0.0;
	}
}


void nlp::Plsa::_allocate_latent_variable()
{
	LOG(INFO) << "Init latent variable" << std::endl;
	_doc_term_probs = new double**[_docs_nums];
	
	#pragma omp parallel for
	for (int i = 0; i < _docs_nums; ++i)
	{
		_doc_term_probs[i] = new double*[_terms_nums];
		for (int j = 0; j < _terms_nums; ++j)
		{
			_doc_term_probs[i][j] = new double[MAX_TOPICS];
		}
	}

	#pragma omp parallel for
	for (int i = 0; i < _docs_nums; ++i)
	{
		for (int j = 0; j < _terms_nums; ++j)
		{
			for (int k = 0; k < MAX_TOPICS; ++k)
			{
				_doc_term_probs[i][j][k] = 0.0;
			}
		}
	}
}

nlp::Plsa::Plsa(std::string &conf_path)
{
	if (!_load_config(conf_path))
	{
		LOG(FATAL)<<"Load configuration failed"<<std::endl;
	}
}


nlp::Plsa::~Plsa()
{
	for (int i = 0; i < _docs_nums; ++i)
		delete _doc_probs[i];
	for (int i = 0; i < _terms_nums; ++i)
		delete _term_probs[i];
	delete _doc_probs;
	delete _term_probs;
}


void nlp::Plsa::_init_latent_variable()
{
	LOG(INFO) << "Init latent variable z" << std::endl;
	srand(time(0));
	LOG(INFO) <<_docs_nums<<" "<<_topics_nums<<" "<<_terms_nums<<std::endl;
	clock_t start, finish;
	double total_time = 0.0;
	start = clock();
	for (int i = 0; i < _docs_nums; ++i)
	{
		#pragma omp parallel for
		for (int j = 0; j < _terms_nums; ++j)
		{
			double lm_prob = 0.0; //背景语言模型概率
			if (_doc_words_map[i] == 0)
				lm_prob = 0.0;
			else
				lm_prob = double(_counts[i][j]) / double(_doc_words_map[i]); //unigram
			if (isnan(lm_prob))
			{
				LOG(FATAL)<<"Language model init error! NAN"<<std::endl;
			}
			_doc_term_probs[i][j][LM] = lm_prob;
			std::vector<int> random_nums;
			int total = 0;
			for (int k = 0; k < _topics_nums; ++k) //TODO(zhounan) 随机初始化这一步效率低，需优化
			{
				int num = rand()%100 + 1;
				random_nums.push_back(num);
				total += num;
			}
			for (int k = 0; k < _topics_nums; ++k)
			{
				double prob = double(random_nums[k]) / double(total);
				if (isnan(prob))
				{
					LOG(FATAL)<<"Latent init error! NAN"<<std::endl;
				}
				_doc_term_probs[i][j][k] = prob;
			}
		}
	}
	finish = clock();
	total_time = (double)(finish-start) / CLOCKS_PER_SEC;
	LOG(INFO) <<"Init latent z finish"<<" "<<total_time<<"s" <<std::endl;

}


void nlp::Plsa::_calc_latent_variable()
{
	LOG(INFO) <<"Calc latent"<<std::endl;
	clock_t start,finish;
	double total_time = 0.0;
	start = clock();
	#pragma omp parallel for
	for (int i = 0; i < _docs_nums; ++i)
	{
		for (int j = 0; j < _terms_nums; ++j)
		{
			double topic_sum = 0.0;
			for (int k = 0; k < _topics_nums; ++k)
			{
				double prob = _doc_probs[i][k] * _term_probs[j][k];
				_doc_term_probs[i][j][k] = prob;
				topic_sum += prob;
			}
			for (int k = 0; k < _topics_nums; ++k)
			{
				if (_doc_term_probs[i][j][k] != 0)
					_doc_term_probs[i][j][k] /= topic_sum;
				if (isnan(_doc_term_probs[i][j][k]))
				{
					LOG(FATAL)<<"Calc latent error! NAN"<<std::endl;
				}
			}
			double lm_prob = _tfs_map[j];
			_doc_term_probs[i][j][LM] = (_lambda * lm_prob) / (_lambda * lm_prob + (1 - _lambda) * topic_sum);
			if (isnan(_doc_term_probs[i][j][LM]))
			{
				LOG(FATAL)<<"Calc latent error! NAN"<<std::endl;
			}
		}
	}
	finish = clock();
	total_time=(double)(finish-start) / CLOCKS_PER_SEC;
	LOG(INFO) <<"Calc latent finish"<<" "<<total_time<<"s"<<std::endl;

}


void nlp::Plsa::_calc_doc_prob()
{
	LOG(INFO) <<"Calc doc prob"<<std::endl;
	clock_t start, finish;
	double total_time = 0.0;
	start = clock();
	#pragma omp parallel for
	for (int i = 0; i < _docs_nums; ++i)
	{
		double topic_sum = 0.0;
		for (int k = 0; k < _topics_nums; ++k)
		{
			double term_sum = 0.0;
			for (int j = 0; j < _terms_nums; ++j)
			{
				term_sum += _counts[i][j] * (1 - _doc_term_probs[i][j][LM]) * _doc_term_probs[i][j][k];
			}
			_doc_probs[i][k] = term_sum;
			topic_sum += term_sum;
		}
		for (int k = 0; k < _topics_nums; ++k)
		{
			if (_doc_probs[i][k] != 0)
				_doc_probs[i][k] /= topic_sum;
			if (isnan(_doc_probs[i][k]))
			{
				LOG(FATAL)<<"Calc doc prob error! NAN"<<std::endl;
			}
		}

	}
	finish = clock();
	total_time=(double)(finish-start) / CLOCKS_PER_SEC;
	LOG(INFO)<<"Calc doc prob finish"<<" "<<total_time<<"s"<<std::endl;
}


void nlp::Plsa::_calc_term_prob()
{
	LOG(INFO) <<"Calc term prob"<<std::endl;
	clock_t start, finish;
	double total_time = 0.0;
	start = clock();
	#pragma omp parallel for
	for (int k = 0; k < _topics_nums; ++k)
	{
		double term_sum = 0.0;
		for (int j = 0; j < _terms_nums; ++j)
		{
			double doc_sum = 0.0;
			for (int i = 0; i < _docs_nums; ++i)
			{
				doc_sum += _counts[i][j] * (1 - _doc_term_probs[i][j][LM]) * _doc_term_probs[i][j][k];
			}
			term_sum += doc_sum;
			_term_probs[j][k] = doc_sum;
		}
		for (int j = 0; j < _terms_nums; ++j)
		{
			if (_term_probs[j][k] != 0)
				_term_probs[j][k] /= term_sum;
			if (isnan(_term_probs[j][k]))
			{
				LOG(FATAL)<<"Calc term prob error! NAN"<<std::endl;
			}
		}
	}
	finish = clock();
	total_time=(double)(finish-start) / CLOCKS_PER_SEC;
	LOG(INFO) <<"Calc term finish"<<" "<<total_time<<"s"<<std::endl;
}


void nlp::Plsa::_init_EM()
{
	std::cout<<"Init EM args"<<std::endl;
	_init_latent_variable();
	_calc_doc_prob();
	_calc_term_prob();
	std::cout<<"Init EM args end"<<std::endl;
}


void nlp::Plsa::_Estep()
{
	_calc_latent_variable();
}


void nlp::Plsa::_Mstep()
{
	_calc_doc_prob();
	_calc_term_prob();
}


void nlp::Plsa::_destroy_intermedian_variable()
{
	for (int i = 0; i < _docs_nums; ++i)
	{
		for (int j = 0; j < _terms_nums; ++j)
			delete _doc_term_probs[i][j];
		delete _doc_term_probs[i];
	}
	for (int i = 0; i < _docs_nums; ++i)
		delete _counts[i];
	
	delete _doc_term_probs;
	delete _counts;
}


bool nlp::Plsa::train_plsa()
{
	LOG(INFO) << "Train PLSA" <<std::endl;
	if (!_preprocess())
	{
		LOG(FATAL) << "Preprocess failed !"<<std::endl;
		return false;
	}
	if (_docs_nums <= 0 || _terms_nums <= 0 || _topics_nums <= 0)
	{
		LOG(FATAL)<<"doc: "<<_docs_nums<<" term: "<<_terms_nums<<" topic: "<<_topics_nums<<std::endl;
		return false;
	}
	LOG(INFO) << "Preprocess succeed " <<"doc:" <<_docs_nums << " " <<"terms: "<< _terms_nums<<std::endl;
	_allocate_latent_variable();
	_init_probs();
	_init_EM();
	for (int i = 0; i < _iter_nums; ++i)
	{
		LOG(INFO) <<i<<"th iteration"<<std::endl;
		std::cout<<i<<"th iteration"<<std::endl;
		_Estep();
		_Mstep();
	}
	std::string save_path = "../data";
	save_probs(save_path);
	_destroy_intermedian_variable();
	return true;
	
}


bool nlp::Plsa::get_topic_words(std::map<int, std::vector<std::string> > &topic_words)
{
	LOG(INFO) << "Get PLSA topic words" << std::endl;
	std::map<int, std::map<double, int> > tmp_word_map; //临时变量　存储话题id与词－tf对的映射
	for (int i = 0; i < _topics_nums; ++i)
	{
		std::vector<std::string> words;
		std::map<double, int> prob_map; //以tf为键可以利用map默认的排序方式实现排序
		topic_words[i] = words;
		tmp_word_map[i] = prob_map;
	}
	
	for (int i = 0; i < _topics_nums; ++i)
	{
		for (int j = 0; j < _terms_nums; ++j)
		{
			if (isnan(_term_probs[j][i]))
			{
				LOG(FATAL)<<"Get PLSA topic error! NAN"<<std::endl;
				return false;
			}
			double prob =  - (_term_probs[j][i]); //map 默认按照key从小到大排序, 取相反数逆序排
			tmp_word_map[i][prob] = j;
		}
	}

	for (int i = 0; i < _topics_nums; ++i)
	{
		std::map<double, int>::iterator iter;
		int word_count = 0;
		for (iter = tmp_word_map[i].begin(); iter != tmp_word_map[i].end(); ++iter)
		{
			if (word_count >= 10) //取每个话题tf前10的词作为话题关键词
				break;
			int id = iter->second;
			string word = _literal_map[id];
			LOG(INFO)<<word<<":"<<iter->first<<" ";
			std::cout<<word<<":"<<iter->first<<" "; //TODO(zhounan) 将话题词输出到屏幕，后期可修改此处
			topic_words[i].push_back(word);
			word_count++;
		}
		std::cout<<std::endl; //TODO(zhounan)
		LOG(INFO)<<std::endl;
	}
	
	LOG(INFO) << "Get PLSA topic words succeed" << std::endl;
	return true;
}


std::map<int, WeiboTopic_ICT::Weibo> nlp::Plsa::get_documents() const
{
	return _id_doc_map;
}


void nlp::Plsa::save_probs(std::string &path)
{
	LOG(INFO) << "Save probs" << std::endl;
	std::string term_path = path + "/term_probs";
	std::string doc_path = path + "/doc_probs";
	std::string dict_path = path + "/dict";

	std::ofstream term_out(term_path.c_str());
	std::ofstream doc_out(doc_path.c_str());
	std::ofstream dict_out(dict_path.c_str());
	
	term_out<<_terms_nums<<" "<<_topics_nums<<std::endl;
	for (int i = 0; i < _terms_nums; ++i)
	{
		term_out<<i<<" ";
		for (int k = 0; k < _topics_nums; ++k)
		{
			term_out<<_term_probs[i][k]<<" ";
		}
		term_out<<std::endl;
	}

	doc_out<<_docs_nums<<" "<<_topics_nums<<std::endl;
	for (int i = 0; i < _docs_nums; ++i)
	{
		doc_out<<i<<" ";
		for (int k = 0; k < _topics_nums; ++k)
		{
			doc_out<<_doc_probs[i][k]<<" ";
		}
		doc_out<<std::endl;
	}

	std::map<int, double>::iterator iter;
	for (iter = _tfs_map.begin(); iter != _tfs_map.end(); ++iter)
	{
		int id = iter->first;
		string word = _literal_map[id];
		dict_out<<id<<" "<<word<<std::endl;
	}
	term_out.close();
	doc_out.close();
	dict_out.close();
}


bool nlp::Plsa::load_probs(std::string &path)
{
	std::string term_path = path + "/term_probs";
	std::string doc_path = path + "/doc_probs";
	std::string dict_path = path + "/dict";
	LOG(INFO) << "Load probs" << std::endl;
	if ((access(term_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Term probs path is wrong" << std::endl;
		return false;
	}
	if ((access(doc_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Doc probs path is wrong" << std::endl;
		return false;
	}
	if ((access(dict_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Dict probs path is wrong" << std::endl;
		return false;
	}
	std::ifstream term_in(term_path.c_str());
	std::ifstream doc_in(doc_path.c_str());
	std::ifstream dict_in(dict_path.c_str());

	int terms_nums, topics_nums;
	term_in >> terms_nums;
	term_in >> topics_nums;
	int docs_nums;
	doc_in >> docs_nums;
	doc_in >> topics_nums;

	_docs_nums = docs_nums;
	_terms_nums = terms_nums;
	_topics_nums = topics_nums;

	_init_probs();

	for (int i = 0; i < terms_nums; ++i)
	{
		int index;
		term_in >> index;
		for (int k = 0; k < topics_nums; ++k)
		{
			double prob;
			term_in >> prob;
			_term_probs[i][k] = prob;
		}
	}

	for (int i = 0; i < docs_nums; ++i)
	{
		int index;
		doc_in >> index;
		for (int k = 0; k < topics_nums; ++k)
		{
			double prob;
			doc_in >> prob;
			_doc_probs[i][k] = prob;
		}
	}

	for (int i = 0; i < _terms_nums; ++i)
	{
		int id;
		string word;
		dict_in >> id >> word;
		_literal_map[id] = word;
	}
	LOG(INFO) << "Load probs succeed" << std::endl;
	return true;
}


void nlp::Plsa::set_documents(std::vector<WeiboTopic_ICT::Weibo> &doc_list)
{
	_id_doc_map.clear();
	for (unsigned int i = 0; i < doc_list.size(); i++)
	{
		_id_doc_map[i] = doc_list[i];
	}
}
