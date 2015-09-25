/*************************************************************************
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
