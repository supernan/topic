#include "ICTSimHashInterface.h"
#include "hash.hpp"
#include "simhash.h"
#include <map>

namespace Simhash
{
	//define handler
	typedef struct Handler
	{
		Table *m_table;							//simtable to store all the hash value
		map<hash_t, vector<int> > indexTable;	//key:hash value value:the index set in corpus

		Handler()
		{
			m_table = NULL;
			indexTable.clear();
		}

		~Handler()
		{
			if(m_table)
				delete m_table;
			indexTable.clear();
		}
	};

	bool split(const string &src, const char *delimit, vector<string> &result)
	{	
		if(0 == src.size() || NULL == delimit)
			return false;
		result.clear();	
		char source[1024000] = {0};
		memcpy(source, src.c_str(), src.size());	
		char *p = strtok(source, delimit);
		while(p)
		{
			string temp = p;
			result.push_back(temp);
			p = strtok(NULL, delimit);
		}
		return true;
	}

	/*!
	*  \fn		void Open(int k, SimhashHandler &m_handler)
	*  \brief	open Simhash handle
	*  \param [in]	k	define number of bits why which a query  may differ from a near-duplicate document
	*  \param [out] m_handler	return Simhash handler
	*  \return		void
	*/
	void Open(int k, SimhashHandler &m_handler)
	{
		if(k < 0 || k > 64)
			return ;

		vector<hash_t> masks;
		/* This is a vector of bitmasks describing a valid permutation. This 
		 * particular permutation has 6 blocks, where each of the blocks is kept in
		 * the order that you'd expect. Changing the order of these items has the
		 * effect of reordering the blocks. */
		masks.push_back(0xFFE0000000000000);
		masks.push_back(0x1FFC0000000000  );
		masks.push_back(0x3FF80000000     );
		masks.push_back(0x7FF00000        );
		masks.push_back(0xFFE00           );
		masks.push_back(0x1FF             );
		
		if(m_handler == NULL)
			m_handler = new Handler;
		else
		{
			delete (Handler*)m_handler;
			m_handler = new Handler;
		}
		((Handler*)m_handler)->m_table = new Table(k, masks);
	}

	/*!
	*  \fn		void SetData(SimhashHandler handler,vector<string> &data)
	*  \brief	compute the corpus hash value and store them
	*  \param [in]	handler	Simhash handler
	*  \param [in] 	data	corpus,format:xx xx xx xx(xx is the word item after wordseg)
	*  \return		void
	*/
	void SetData(SimhashHandler handler, vector<string> &data)
	{
		Simhash<> hasher;
		Handler *m_handler = (Handler*)handler;
		for(int i = 0; i < data.size(); ++i)
		{
			vector<string> res;
			split(data[i], " ", res);		//get word seg
			char **src = new char*[res.size()+1];
			for(int i = 0; i < res.size(); ++i)
				src[i] = const_cast<char*>(res[i].c_str());
			src[res.size()] = NULL;			//important!!!
			hash_t hashValue = hasher(src);	//compute hash value
			m_handler->m_table->insert(hashValue);
			(m_handler->indexTable)[hashValue].push_back(i);
			delete[] src;
		}
	}
	
	/*!
	*  \fn		unsigned long HashValue(string &text)
	*  \brief	get the hash value of the text
	*  \param [in]	text	the text to compute the hash value
	*  \return		hash value
	*/
	unsigned long HashValue(string &text)
	{
		Simhash<> hasher;
		vector<string> res;
		split(text, " ", res);		//get word seg
		char **src = new char*[res.size()+1];
		for(int i = 0; i < res.size(); ++i)
			src[i] = const_cast<char*>(res[i].c_str());
		src[res.size()] = NULL;			//important!!!
		hash_t hashValue = hasher(src);	//compute hash value
		delete[] src;
		return hashValue;
		
	}

	/*!
	*  \fn		bool IsExist(SimhashHandler handler,string &query)
	*  \brief	Find if the query is near-duplicate with the corpus
	*  \param [in]	handler	Simhash handler
	*  \param [in] 	query	a query,format:xx xx xx xx(xx is the word item after wordseg)
	*  \return		if exist return true,else return false
	*/
	bool IsExist(SimhashHandler handler, string &query)
	{
		Simhash<> hasher;
		Handler *m_handler = (Handler*)handler;		
		vector<string> res;
		split(query, " ", res);
		char **src = new char*[res.size()+1];
		for(int i = 0; i < res.size(); ++i)
			src[i] = const_cast<char*>(res[i].c_str());
		src[res.size()] = NULL;
		hash_t hashValue = hasher(src);
		delete[] src;
		hash_t i = m_handler->m_table->find(hashValue);
		if(i)
			return true;
		else
			return false;				
	}

	/*!
	*  \fn		void Find(SimhashHandler handler, string &query, vector<int> &result)
	*  \brief	Find all the near duplicates with the query
	*  \param [in]	handler	Simhash handler
	*  \param [in]	query 	the query to find,format:xx xx xx(xx is the word item after wordseg)
	*  \param [out] result	all the index of the corpus of the near duplicates
	*  \return		void
	*/
	void Find(SimhashHandler handler, string &query, vector<int> &result)
	{
		result.clear();
		Simhash<> hasher;
		Handler *m_handler = (Handler*)handler;		
		vector<string> res;
		split(query, " ", res);
		char **src = new char*[res.size()+1];
		for(int i = 0; i < res.size(); ++i)
			src[i] = const_cast<char*>(res[i].c_str());
		src[res.size()] = NULL;
		hash_t hashValue = hasher(src);
		delete[] src;
		vector<hash_t> searchRes;
		m_handler->m_table->find(hashValue, searchRes);
		for(int i = 0; i < searchRes.size(); ++i)
		{
			vector<int> index = (m_handler->indexTable)[searchRes[i]];
			for(int j = 0; j < index.size(); ++j)
				result.push_back(index[j]);
		}		
	}

	/*!
	*  \fn		void Destroy(SimhashHandler &handler)
	*  \brief	destroy the handler
	*  \param [in]	handler	Simhash handler
	*  \return		void
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	void Destroy(SimhashHandler &handler)
	{
		Handler *m_handler = (Handler*)handler;
		if(m_handler)
		{
			delete m_handler;
			m_handler = NULL;
		}
	}
}
