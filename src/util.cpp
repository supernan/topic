/*************************************************************************
 > File Name: ../src/util.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 10时29分07秒
 ************************************************************************/

#include"util.h"

bool tools::Util::read_text_from_local(std::string &local_path, std::map<int, WeiboTopic_ICT::Weibo> &id_doc_map)
{
	if ((access(local_path.c_str(), F_OK)) == -1)
	{
		LOG(FATAL) << "text local path is wrong" << std::endl;
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
				LOG(FATAL) << "text format error: " <<doc_count<<" "<< line <<std::endl;
				return false;
			}
			std::string text = parts[1];
			std::string utc_time_str = parts[0];
			double utc_time = atof(utc_time_str.c_str());
			doc.index = doc_count;
			doc.mt = text;
			doc.pt = utc_time;
			id_doc_map[doc.index] = doc;
			doc_count += 1;
		}
		else
		{
			LOG(FATAL) << "text format error: " <<doc_count<<" "<< line <<std::endl;
			return false;
		}

	}
	return true;
}


bool tools::Util::split_line(std::string &line, std::string &key, std::vector<std::string> &word_list)
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


void tools::Util::read_words_dict(std::string &path, std::map<int, double> &dict_tf)
{
	if ((access(path.c_str(), F_OK)) == -1)
		LOG(FATAL) << "dict path is wrong" << std::endl;
	std::ifstream input_stream(path.c_str());
	std::string line;
	int count = 0;
	while (getline(input_stream, line))
	{
		std::vector<std::string> parts;
		std::string key = " ";
		split_line(line, key, parts);
		if (parts.size() < 3)
			LOG(FATAL) << "dict file format is wrong" <<" "<<"line: "<<count<< std::endl;
		int word_id = atoi(parts[0].c_str());
		double tf = atof(parts[2].c_str());
		dict_tf[word_id] = tf;
		count++;
	}
	terms_num = count;
}


void tools::Util::read_doc_tf(std::string &path, double **counts,
		                      std::map<int, double> &doc_words_map, std::map<int, double> &dict_map)
{
	if ((access(path.c_str(), F_OK)) == -1)
		LOG(FATAL) << "doc path is wrong" << std::endl;
	std::ifstream input_stream(path.c_str());
	std::string line;
	docs_num = 0;
	while (getline(input_stream, line))
	{
		std::vector<std::string> parts;
		std::string first_key = " ";
		split_line(line, first_key, parts);
		if (parts.size() != 2)
			LOG(FATAL) << "doc file format is wrong :parts size error" <<" "<<"line: "<<docs_num<< std::endl;
		int doc_id = atoi(parts[0].c_str());
		double total_words = 0;

		std::vector<std::string> records;
		std::string second_key = "|";
		split_line(parts[1], second_key, records);

		for (int i = 0; i < records.size(); i++)
		{
			std::string third_key = ":";
			std::vector<std::string> tfs;
			split_line(records[i], third_key, tfs);
			if (tfs.size() != 2)
				LOG(FATAL) << "doc file format is wrong :tfs size error" << std::endl;
			else
			{
				int term_id = atoi(tfs[0].c_str());
				double tf = atof(tfs[1].c_str());
				total_words += tf;

				std::map<int, double>::iterator iter;
				iter = dict_map.find(term_id);
				if (iter != dict_map.end())
					counts[doc_id][term_id] = tf;
			}

		}
		docs_num++;
		doc_words_map[doc_id] = total_words;
	}
}


void tools::AC_automation::_insert(Trie *root, std::string &pattern)
{
	Trie *cur = root;
	for (int i = 0; i < pattern.length(); i++)
	{
		if (cur->next.find(pattern[i]) == cur->next.end())
			cur->next[pattern[i]] = new Trie();
		cur = cur->next[pattern[i]];
	}
	++cur->count;
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
	_root = new Trie();
	for (int i = 0; i < patterns.size(); i++)
	{
		_insert(_root, patterns[i]);
	}
	_build(_root);
}


int tools::AC_automation::query(std::string &text)
{
	int ret = 0;
	Trie *cur = _root;
	for (int i = 0; i < text.length(); i++)
	{
		while (cur->next[text[i]] == NULL && cur != _root)
			cur = cur->fail;
		cur = cur->next[text[i]];
		if (cur == NULL)
			cur = _root;
		Trie *tmp = cur;
		while (tmp != _root && tmp)
		{
			ret += tmp->count;
			tmp = tmp->fail;
		}
	}
	return ret;
}
