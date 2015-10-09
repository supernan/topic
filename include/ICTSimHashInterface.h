#include <string>
#include <vector>
using namespace std;

namespace Simhash
{
	//difine handler
	typedef void* SimhashHandler;

	/*!
	*  \fn		void Open(int k, SimhashHandler &m_handler)
	*  \brief	open Simhash handle
	*  \param [in]	k	define number of bits why which a query  may differ from a near-duplicate document
	*  \param [out] m_handler	return Simhash handler
	*  \return		void
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	void Open(int k, SimhashHandler &m_handler);

	/*!
	*  \fn		void SetData(SimhashHandler handler,vector<string> &data)
	*  \brief	compute the corpus hash value and store them
	*  \param [in]	handler	Simhash handler
	*  \param [in] 	data	corpus,format:xx xx xx xx(xx is the word item after wordseg)
	*  \return		void
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	void SetData(SimhashHandler handler, vector<string> &data);
	
	/*!
	*  \fn		unsigned long HashValue(string &text)
	*  \brief	get the hash value of the text
	*  \param [in]	text	the text to compute the hash value
	*  \return		hash value
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-9-25
	*/
	unsigned long HashValue(string &text);

	/*!
	*  \fn		bool IsExist(SimhashHandler handler,string &query)
	*  \brief	Find if the query is near-duplicate with the corpus
	*  \param [in]	handler	Simhash handler
	*  \param [in] 	query	a query,format:xx xx xx xx(xx is the word item after wordseg)
	*  \return		if exist return true,else return false
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	bool IsExist(SimhashHandler handler, string &query);

	/*!
	*  \fn		void Find(SimhashHandler handler, string &query, vector<int> &result)
	*  \brief	Find all the near duplicates with the query
	*  \param [in]	handler	Simhash handler
	*  \param [in]	query 	the query to find,format:xx xx xx(xx is the word item after wordseg)
	*  \param [out] result	all the index of the corpus of the near duplicates
	*  \return		void
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	void Find(SimhashHandler handler, string &query, vector<int> &result);

	/*!
	*  \fn		void Destroy(SimhashHandler handler)
	*  \brief	destroy the handler
	*  \param [in]	handler	Simhash handler
	*  \return		void
	*  \author	chenqiang(chenqiang@software.ict.ac.cn)
	*  \date	2014-6-24
	*/
	void Destroy(SimhashHandler &handler);
}
