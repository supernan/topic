/*************************************************************************
 *
 > File Name: ../src/util.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 10时29分07秒
 ************************************************************************/

#include"util.h"

bool tools::UtilInterface::read_text_from_local(std::string &local_path, std::vector<WeiboTopic_ICT::Weibo> &doc_list)
{
	if ((access(local_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "Text local path is wrong" << std::endl;
		return false;
	}

	std::ifstream input_stream(local_path.c_str());
	std::string line;
	std::string sep = SEP_FLG;
	int doc_count = 0;
	while (getline(input_stream, line))
	{
		WeiboTopic_ICT::Weibo doc;
		std::vector<std::string> parts;
		if (split_line(line, sep, parts))
		{
			if (parts.size() < 2)
			{
				LOG(FATAL) << "Text format error: " <<doc_count<<" "<< line <<std::endl;
				return false;
			}
			std::string text = parts[1];
			std::string utc_time_str = parts[0];
			double utc_time = atof(utc_time_str.c_str());
			doc.index = doc_count;
			doc.mt = text;
			doc.pt = utc_time;
			doc_list.push_back(doc);
			doc_count += 1;
		}
		else
		{
			LOG(FATAL) << "Text format error: " <<doc_count<<" "<< line <<std::endl;
			return false;
		}

	}
	return true;
}


bool tools::UtilInterface::split_line(std::string &line, std::string &key, std::vector<std::string> &word_list)
{
	int pos = -1;
	int last_pos = 0;
	int key_size = key.length();
	int line_size = line.length();
	if (key_size == 0)
	{
		LOG(ERROR) << "_split_line key size is 0" << std::endl;
		return false;
	}
	if (line_size == 0)
	{
		LOG(ERROR) << "_split_line line size is 0" << std::endl;
		return false;
	}
	
	while (true)
	{
		pos = line.find(key, last_pos);
		if (pos == -1)
			break;
		std::string word = line.substr(last_pos, pos - last_pos);
		if (word != "")
			word_list.push_back(word);
		last_pos = pos + key_size;
	}
	std::string last_word = line.substr(last_pos);
	if (last_word != "")
		word_list.push_back(last_word);
	return true;
}


tools::AC_automation::AC_automation()
{
	_root = new Trie();
}


void tools::AC_automation::_destroy_tree(Trie *root)
{
	if (root == NULL) //空指针 判断
		return;
	std::map<char, Trie*>::iterator iter;
	for (iter = root->next.begin(); iter != root->next.end(); ++iter)
	{
		_destroy_tree(iter->second);
	}
	delete root;
}


tools::AC_automation::~AC_automation()
{
	_destroy_tree(_root);
}


void tools::AC_automation::_insert(Trie *root, std::string &pattern, int id)
{
	Trie *cur = root;
	for (unsigned int i = 0; i < pattern.length(); ++i)
	{
		if (cur->next.find(pattern[i]) == cur->next.end())
			cur->next[pattern[i]] = new Trie();
		cur = cur->next[pattern[i]];
	}
	++cur->count;
	cur->pattern_id = id;
}


void tools::AC_automation::_build(Trie *root)
{
	std::queue<Trie*> trie_queue;
	while (!trie_queue.empty())
		trie_queue.pop();
	trie_queue.push(root);
	while (!trie_queue.empty())
	{
		Trie *cur = trie_queue.front();
		trie_queue.pop();
		std::map<char, Trie*>::iterator iter;
		for (iter = cur->next.begin(); iter != cur->next.end(); ++iter)
		{
			if (cur == root)
				cur->next[iter->first]->fail = root;
			else
			{
				Trie *tmp = cur->fail;
				while (tmp)
				{
					if (tmp->next[iter->first])
					{
						cur->next[iter->first]->fail = tmp->next[iter->first];
						break;
					}
					tmp = tmp->fail;
				}
				if (tmp == NULL)
					cur->next[iter->first]->fail = root;
			}
			trie_queue.push(cur->next[iter->first]);
		}
	}
}


void tools::AC_automation::build_automation(std::vector<std::string> &patterns)
{
	for (unsigned int i = 0; i < patterns.size(); ++i)
	{
		_visit[i] = 0;
		_insert(_root, patterns[i], i);
		_id_pattern_map[i] = patterns[i];
	}
	_build(_root);
}


std::map<int, std::string> tools::AC_automation::query(std::string &text)
{
	_visit.clear(); //TODO(zhounan) 全赋值为0 or 清空
	int ret = 0;
	std::map<int, std::string> match_patterns;
	Trie *cur = _root;
	for (unsigned int i = 0; i < text.length(); ++i)
	{
		while (cur->next[text[i]] == NULL && cur != _root)
			cur = cur->fail;
		cur = cur->next[text[i]];
		if (cur == NULL)
			cur = _root;
		Trie *tmp = cur;
		while (tmp != _root && tmp)
		{
			if (tmp->pattern_id != -1 &&_visit[tmp->pattern_id] == 0)
			{
				ret += tmp->count;
				_visit[tmp->pattern_id] = 1;
				match_patterns[tmp->pattern_id] = _id_pattern_map[tmp->pattern_id];
			}
			tmp = tmp->fail;
		}
	}
	return match_patterns;
}


void tools::AC_automation::clear()
{
	_destroy_tree(_root);
	_root = new Trie();
	_id_pattern_map.clear();
	_visit.clear();
}


int tools::RelationTreeInterface::find_parent(int pos, std::vector<int> &parents)
{
	if (pos != parents[pos])
		parents[pos] = find_parent(parents[pos], parents);
	return parents[pos];
}


void tools::RelationTreeInterface::union_node(int x, int y, std::vector<int> &parents)
{
	int x_parent = find_parent(x, parents);
	int y_parent = find_parent(y, parents);
	if (x_parent == y_parent)
		return;
	if (x > y)
		parents[y] = x;
	else
		parents[x] = y;
}


tools::WordSegThreadPool::WordSegThreadPool(int thread_num, std::map<int, WeiboTopic_ICT::Weibo> &id_doc_map)
{
	WordSegThreadPool::_thread_num = thread_num;
	WordSegThreadPool::_doc_num = id_doc_map.size();
	if (!NLPIR_Init("../data/", UTF8_CODE))
	{
		LOG(FATAL) << "ICTCLAS init error" <<std::endl;
	}
	
	std::map<int, WeiboTopic_ICT::Weibo>::iterator iter;
	for (iter = id_doc_map.begin(); iter != id_doc_map.end(); ++iter)
	{
		_doc_ids.push_back(iter->first);
		_docs.push_back(iter->second.mt);
	}
}


void* tools::_word_cut_thread(void *arg)
{
	std::map<int, std::string> index_rawtext_map;
	std::map<std::string, int> tmp_tf_map;
	thread_param *param = (thread_param*)arg;
	int start = param->start;
	int end = param->end;
	std::string sep = " ";
	thread_ret *ret = new thread_ret;
	for (unsigned int i = start; i <= end; ++i)
	{
		const char *result;
		int id = param->ws_ptr->_doc_ids[i];
		std::string text = param->ws_ptr->_docs[i];
		result = NLPIR_ParagraphProcess(text.c_str(), 0);
		std::string raw_line = result;
		index_rawtext_map[id] = raw_line; //保留文档分词后的结果
		std::vector<std::string> words;
		if (!tools::UtilInterface::split_line(raw_line, sep, words))
		{
			LOG(ERROR) << "Error when cut words " <<std::endl;
		}
		for (unsigned int i = 0; i < words.size(); ++i)
			tmp_tf_map[words[i]]++; //词频统计
		
	}
	ret->local_rawtext_map = index_rawtext_map;
	ret->local_tf_map = tmp_tf_map;
	return (void*)ret;
}


void tools::WordSegThreadPool::_merge_local_map(thread_ret *ret, std::map<int, std::string> &id_rawtext_map,
		                                        std::map<std::string, int> &tmp_tf_map)
{
	std::map<int, std::string> raw_text_map = ret->local_rawtext_map;
	std::map<std::string, int> tf_map = ret->local_tf_map;
	std::map<int, std::string>::iterator text_iter;
	std::map<std::string, int>::iterator tf_iter;
	for (text_iter = raw_text_map.begin(); text_iter != raw_text_map.end(); ++text_iter)
	{
		int id = text_iter->first;
		std::string text = text_iter->second;
		id_rawtext_map[id] = text;
	}
	for (tf_iter = tf_map.begin(); tf_iter != tf_map.end(); ++tf_iter)
	{
		std::string word = tf_iter->first;
		int tf = tf_iter->second;
		tmp_tf_map[word] += tf;
	}
}


void tools::WordSegThreadPool::_destroy_word_seg(void **ret, pthread_t* tids, thread_param* params)
{
	for (int i = 0; i < _thread_num; ++i)
		delete ret[i];
	delete ret;
	delete tids;
	delete params;
	NLPIR_Exit();
}


bool tools::WordSegThreadPool::multithread_word_cut(std::map<int, std::string> &id_rawtext_map,
		                         std::map<std::string, int> &tmp_tf_map)
{
	LOG(INFO)<<"Multi thread word cut begin"<<std::endl;
	int each_thread_docs = _doc_num / _thread_num;
	pthread_t *tids = new pthread_t[_thread_num];
	void **rets = new void*[_thread_num];
	thread_param *params = new thread_param[_thread_num];
	if (each_thread_docs == 0)
	{
		LOG(ERROR)<<"Docs is too few less than thread_num!"<<std::endl;
		return false;
	}
	params[0].start = 0;
	params[0].end = each_thread_docs - 1;
	params[0].ws_ptr = this;
	for (int i = 1; i < _thread_num; ++i)
	{
		params[i].start = params[i-1].end + 1;
		params[i].ws_ptr = this;
		if ((params[i].start + each_thread_docs-1) >_doc_num-1)
			params[i].end = _doc_num - 1;
		else
			params[i].end = params[i].start + each_thread_docs -1;
	}
	for (int i = 0; i < _thread_num; ++i)
	{
		pthread_create(&tids[i], NULL, _word_cut_thread, (void*)(&params[i]));
	}
	for (int i = 0; i < _thread_num; ++i)
	{
		pthread_join(tids[i], &rets[i]);
		thread_ret *ws_ret = (thread_ret*)rets[i];
		_merge_local_map(ws_ret, id_rawtext_map, tmp_tf_map);
	}
	_destroy_word_seg(rets, tids, params);
	LOG(INFO)<<"Multi thread word cut end"<<std::endl;
	return true;
}
