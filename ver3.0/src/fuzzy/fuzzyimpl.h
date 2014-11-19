/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2010 The Terimber Corporation. All rights reserved.
 * =================================================================================
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * The end-user documentation included with the redistribution, if any, 
 * must include the following acknowledgment:
 * "This product includes software developed by the Terimber Corporation."
 * =================================================================================
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL THE TERIMBER CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ================================================================================
*/

#ifndef _terimber_fuzzyimpl_h_
#define _terimber_fuzzyimpl_h_

#include "fuzzy/fuzzyaccess.h"

#include "base/map.h"
#include "base/vector.h"
#include "base/stack.h"
#include "base/string.h"
#include "smart/byterep.h"
#include "tokenizer/tokenizer.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)


#define MAX_TOKEN_LENGTH	(size_t)64
#define MAX_PHRASE_TOKENS	(size_t)64

//! \class metaphone_key
//! \brief metaphone key
class metaphone_key
{
public:
	//! \brief constructor
	metaphone_key() : 
		_array(0), 
		_length(0) 
	{
	}

	//! \brief compare operator
	inline 
	bool 
	operator<(const metaphone_key& x) const
	{
		if (!_length)
			return x._length != 0;
		else if (!x._length)
			return false;
		else
		{
			ub4_t min_len = __min(_length, x._length);
			int res = memcmp(_array, x._array, min_len);
			return res != 0 ? res < 0 : _length < x._length;
		}
	}

	ub1_t*	_array;											//!< binary array - metaphoneic codes
	ub4_t	_length;										//!< length of array
};

//! \class metaphone_entry 
//! \brief metaphone entry
class metaphone_entry
{
public:
	//! \brief constructor
	metaphone_entry(size_t mpk								//!< metaphone key
					) : 
		_mpk(mpk), 
		_ref(1) 
	{
	}

	
	size_t	_mpk;											//!< metaphone key
	size_t	_ref;											//!< reference counter
};

//! \class reflection_key
//! \brief reflection key
class reflection_key
{
public:
	//! \enum reflection_size
	//! \brief reflection size
	enum reflection_size
	{
		REFSIZE = 17,										//!< max reflection size
		REFMAX = 0xFF										//!< max reflection value
	};

	//! \typedef reflection_bytes_t
	//! \brief reflection array of bytes
	typedef ub1_t reflection_bytes_t[REFSIZE];

public:
	//! \brief constructor
	reflection_key() 
	{
		memset(_array, 0, REFSIZE * sizeof(ub1_t));
	}

	//! \brief compare operator
	inline 
	bool 
	operator<(const reflection_key& x) const
	{
		return memcmp(_array, x._array, REFSIZE * sizeof(ub1_t)) < 0;
	}

	reflection_bytes_t				_array;					//!< binary array - metaphoneic codes
};

//! \class reflection_entry
//! \brief reflection entry
class reflection_entry
{
public:
	//! \brief constructor
	reflection_entry(size_t rpk) : 
	_rpk(rpk), 
	_ref(1) 
	{
	}

	
	size_t	_rpk;											//!< reflection key
	size_t	_ref;											//!< reference counter
};


//! \typedef metaphone_entry_t
//! \brief metaphone map key -> entry
typedef map< metaphone_key, metaphone_entry >	metaphone_entry_t;
//! \typedef metaphone_entry_iter_t
//! \brief iterator of metaphone_entry_t
typedef metaphone_entry_t::iterator				metaphone_entry_iter_t;
//! \typedef metaphone_entry_citer_t
//! \brief const iterator of metaphone_entry_citer_t
typedef metaphone_entry_t::const_iterator		metaphone_entry_citer_t;

//! \typedef reflection_entry_t
//! \brief n-gram compound metaphoneic reflection -> reflaction entry
typedef map< reflection_key, reflection_entry >	reflection_entry_t;
//! \typedef reflection_entry_iter_t
//! \brief iterator of reflection_entry_iter_t
typedef reflection_entry_t::iterator			reflection_entry_iter_t;
//! \typedef reflection_entry_citer_t
//! \brief conbst iterator of reflection_entry_iter_t
typedef reflection_entry_t::const_iterator		reflection_entry_citer_t;

//! \class word_key
//! \brief a single word key
class word_key
{
public:
	//! \brief constructor
	word_key(		const char* str,						//!< string
					size_t len								//!< string length
					) : 
		_str(str), 
		_len(len) 
	{
	}

	//! \brief compare operator
	inline 
	bool 
	operator<(const word_key& x) const
	{
		if (!_str)
			return x._str != 0;
		else if (!x._str)
			return false;
		else
		{
			size_t min_len = __min(_len, x._len);
			int res = str_template::strnocasecmp(_str, x._str, min_len);
			return res != 0 ? res < 0 : _len < x._len;
		}
	}

	const char* 		_str;								//!< string pointer
	size_t				_len;								//!< string length
};

//! \class word_entry
//! \brief a sigle word information
class word_entry
{
public:
	//! \brief constructor
	word_entry(		size_t vpk,								//!< work key 
					metaphone_entry_iter_t miter			//!< iterator to the correspondent metaphone entry
					) : 
		_vpk(vpk), 
		_miter(miter), 
		_ref(1)
	{
	}

	size_t						_vpk;						//!< work key
	metaphone_entry_iter_t		_miter;						//!< iterator to the correspondent metaphone entry
	size_t						_ref;						//!< reference counter 
};

//! \typedef word_entry_t
//! \brief vocabulary (origin -> internal pk)
typedef map< word_key, word_entry >								word_entry_t;
//! \typedef word_entry_iter_t
//! \brief iterator of word_entry_t
typedef word_entry_t::iterator									word_entry_iter_t;

//! \typedef vpk_word_entry_iter_t
//! \brief reverse map for vpk -> iterator for finding original word
typedef map< size_t, word_entry_iter_t >						vpk_word_entry_iter_t;

//! \typedef mpk_word_entry_iter_t
//! \brief reverse multimap for mpk -> vocabulary_entry iterator for finding original word
typedef map< size_t, word_entry_iter_t, less< size_t >, true >	mpk_word_entry_iter_t;

//! \class ngram_key
//! \brief ngram key
class ngram_key
{
public:
	//! \brief compare operator 
	inline 
	bool 
	operator<(const ngram_key& x) const
	{
		if (!_length)
			return x._length != 0;
		else if (!x._length)
			return false;
		else
		{
			ub4_t min_len = (ub4_t)__min(_length, x._length);
			int res = memcmp(_array, x._array, min_len * sizeof(size_t));
			return res != 0 ? res < 0 : _length < x._length;
		}
	}

	//! \brief equal operator
	inline 
	bool 
	operator==(const ngram_key& x) const
	{
		if (_length != x._length)
			return false;
		else
			return memcmp(_array, x._array, _length * sizeof(size_t)) == 0;
	}

	size_t*				_array;								//!< n-gram keys array
	size_t				_length;							//!< number of keys
};

//! \class ngram_entry
//! \brief ngram entry
class ngram_entry
{
public:
	//! \brief constructor
	ngram_entry(	size_t npk,								//!< n-gram key
					reflection_entry_iter_t riter			//!< iterator to the correspondent reflection
					) : 
		_npk(npk), 
		_riter(riter), 
		_ref(1)
	{
	}
	
	size_t						_npk;						//!< n-gram key
	reflection_entry_iter_t		_riter;						//!< iterator to the correspondent reflection
	ngram_key					_origin;					//!< original (non-sorted) n-gram keys
	size_t						_ref;						//!< reference counter
};

//! \typedef ngram_entry_t
//! \brief n-gram main dictionary (vocabulary idents -> reference counter)
typedef map< ngram_key, ngram_entry >							ngram_entry_t;
//! \typedef ngram_entry_iter_t
//! \brief iterator to ngram_entry_t
typedef ngram_entry_t::iterator									ngram_entry_iter_t;
//! \typedef npk_ngram_entry_iter_t
//! \brief reverse map for ngrams pk -> ngram_entry iterator
typedef map< size_t, ngram_entry_iter_t >						npk_ngram_entry_iter_t;
//! \typedef rpk_ngram_entry_iter_t
//! \brief reverse map for reflection pk -> ngram_entry iterator
typedef map< size_t, ngram_entry_iter_t, less< size_t >, true >	rpk_ngram_entry_iter_t;

//! \class ngram_suggestions_info
//! \brief ngram suggestion information
class ngram_suggestions_info
{
public:
	//! \brief constructor
	ngram_suggestions_info(size_t vpk,						//!< word key
					size_t popularity,						//!< word popularity
					double score,							//!< match score
					size_t penalty							//!< mismatch penalty
					) :
		_vpk(vpk), 
		_popularity(popularity), 
		_score(score), 
		_penalty(penalty)
	{
	}

	size_t		_vpk;										//!< word key
	size_t		_popularity;								//!< word popularity
	double		_score;										//!< match score
	size_t		_penalty;									//!< mismatch penalty
};

//! \typedef sorted_ngram_suggestions_t
//! \brief maps score to the ngram suggestion info
typedef _map< double, ngram_suggestions_info > sorted_ngram_suggestions_t;
//! \typedef sorted_ngram_suggestions_citer_t
//! \brief const iterator of sorted_ngram_suggestions_t
typedef sorted_ngram_suggestions_t::const_iterator sorted_ngram_suggestions_citer_t;

//! \class ngram_suggestions_iter_info
//! \brief ngram suggestion info iterator bag
class ngram_suggestions_iter_info
{
public:
	//! \brief constructor
	ngram_suggestions_iter_info(sorted_ngram_suggestions_citer_t begin, //!< begin iterator
					sorted_ngram_suggestions_citer_t end	//!< end interator
					) :	
		_begin(begin), 
		_current(begin), 
		_end(end)
	{
	}

	sorted_ngram_suggestions_citer_t	_begin;				//!< begin iterator
	sorted_ngram_suggestions_citer_t	_current;			//!< current iterator
	sorted_ngram_suggestions_citer_t	_end;				//!< end iterator
};

//! \typedef ngram_suggestions_iter_list_t
//! \brief list of suggestion info iterator bags
typedef _list< ngram_suggestions_iter_info > ngram_suggestions_iter_list_t;

//! \class string_desc
//! \brief string description
class string_desc
{
public:
	size_t			_vpk;									//!< word key
	double			_score;									//!< match score
	size_t			_popularity;							//!< word popularity
	const char*		_str;									//!< word string
	size_t			_len;									//!< string length
};

//! \class fuzzy_matcher_impl
//! \brief fuzzy match library implementation
class fuzzy_matcher_impl : public fuzzy_matcher
{
	//! \class ngram_key_offset
	//! \brief n-gram ley offset
	class ngram_key_offset
	{
	public:
		//! \brief constructor
		ngram_key_offset(size_t offset,						//!< offset
					ngram_entry_iter_t iter					//!< n-gram iterator
					) :
			_offset(offset), 
			_iter(iter), 
			_pointer(0)
		{
		}
		
		//! \brief constructor
		ngram_key_offset(size_t offset,						//!< offset
					const ngram_key* pointer				//!< external key pointer
					) :
			_offset(offset), 
			_pointer(pointer)
		{
		}

		//! \brief compare operator
		inline 
		bool 
		operator<(const ngram_key_offset& x) const
		{
			if (_offset != x._offset)
				return _offset < x._offset;
			else
			{
				const ngram_key* this_ptr = 0;
				const ngram_key* x_ptr = 0;
				if (x._pointer)
				{
					assert(_pointer == 0);
					this_ptr = &_iter.key();
					x_ptr = x._pointer; 
				}
				else if (_pointer)
				{
					assert(x._pointer == 0);
					this_ptr = _pointer;
					x_ptr = &x._iter.key();
				}
				else
				{
					assert(_pointer == 0);
					assert(x._pointer == 0);
					this_ptr = &_iter.key();
					x_ptr = &x._iter.key();
				}

				ub4_t min_len = (ub4_t)__min(this_ptr->_length, x_ptr->_length);
				assert(min_len > _offset);
				int res = memcmp(this_ptr->_array + _offset, x_ptr->_array + _offset, (min_len - _offset) * sizeof(size_t));
				return res != 0 ? res < 0 : this_ptr->_length < x_ptr->_length;
			}
		}

		size_t					_offset;					//!< offset
		ngram_entry_iter_t		_iter;						//!< n-gram entry iterator
		const ngram_key*		_pointer;					//!< external key pointer
	};

	//! \typedef ngram_key_offset_multimap_t
	//! \brief partial offset map for n-grams
	typedef map< ngram_key_offset, bool, less< ngram_key_offset >, true > ngram_key_offset_multimap_t;

	//! \class candidate_info
	//! \brief match candidate information
	class candidate_info
	{
	public:
		//! \class candidate_info
		//! \brief constructor
		candidate_info(double total_score,					//!< total score
					double intersection_score,				//!< n-gram intersection score
					size_t population,						//!< popularity
					size_t word_penalty						//!< word mismatch penalty
					) :
			_total_score(total_score), 
			_intersection_score(intersection_score), 
			_population(population), 
			_word_penalty(word_penalty)
		{
		}

		double		_total_score;							//!< total score
		double		_intersection_score;					//!< n-gram intersection score
		size_t		_population;							//!< popularity
		size_t		_word_penalty;							//!< word mismatch penalty
	};

	//! \typedef candidates_container_t
	//! \brief maps n-gram key to the candidate information
	typedef _map< ngram_key, candidate_info > candidates_container_t;
	//! \typedef candidates_container_citer_t
	//! \brief const iterator of candidates_container_t
	typedef candidates_container_t::const_iterator candidates_container_citer_t;
	//! \typedef vector_container_citer_t
	//! \brief vector of iterators
	typedef _vector< candidates_container_citer_t > vector_container_citer_t;

	//! \class candidate_sorter
	//! \brief candidate sort predicate
	class candidate_sorter
	{
		//! \class candidate_iter_less
		//! \brief less predicate
		class candidate_iter_less
		{
		public:
			//! \brief operator()
			bool 
			operator()(const candidates_container_citer_t& x, const candidates_container_citer_t& y) const
			{
				return x->_total_score < y->_total_score;
			}
		};


	public:
		//! \brief constructor
		candidate_sorter(const candidates_container_t& container, //!< input container
					vector_container_citer_t& vec,			//!< [out] sorted vector of iterators
					byte_allocator& tmp						//!< external temporary allocator
					)
		{
			size_t len = container.size();
			if (len > 1)
			{
				vec.resize(tmp, len);
				size_t index = 0;
				for (candidates_container_citer_t iter = container.begin(); iter != container.end(); ++iter, ++index)
				{
					vec[index] = iter;
				}

				candidate_iter_less pred;
				std::sort(vec.begin(), vec.end(), pred);

			}
			else if (len > 0)
			{
				vec.resize(tmp, 1);
				vec[0] = container.begin();
			}
		}
	};

public:
	//! \brief constructor
	fuzzy_matcher_impl(size_t memory_usage					//!< max memory usage
					);

	//! \brief destructor
	virtual 
	~fuzzy_matcher_impl();

	// methods

	//! \brief adds a new n-gram to the internal repository (utf-8)
	//! caller can add the same n-gram many times
	//! class will support internal references count
	virtual 
	size_t 
	add(			const char* phrase,						//!< input phrase
					byte_allocator& all			//!< external allocator
					);
	
	//! \brief removes the previously added ngram
	//! caller can remove the same n-gram many times unless references count goes to zero
	virtual 
	bool 
	remove(			const char* phrase,						//!< input phrase
					byte_allocator& all			//!< external allocator
					);
	//! \brief removes the previously added ngram by ident
	//! caller can remove the same n-gram many times unless references count goes to zero
	virtual 
	bool 
	remove(			size_t ident,							//!< input ident
					byte_allocator& all			//!< external allocator
					);
	//! \brief does the fuzzy match
	virtual 
	bool 
	match(			ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality pq,					//!< phonetic quality for matching
					const char* phrase,						//!< input phrase
					byte_allocator& all,			//!< external allocator for output container
					byte_allocator& tmp,			//!< external temporary allocator
					_list< const char* >& suggestions //!< [out] output list of suggestions
					) const;

	//! \brief does the fuzzy match
	virtual 
	bool match(		ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality fq,					//!< phonetic quality for matching
					const char* phrase,						//!< input phrase
					byte_allocator& all,			//!< external allocator for output container
					byte_allocator& tmp,			//!< external temporary allocator
					_list< size_t >& suggestions	//!< [out] output list of sugestions idents
					) const;

	//! \brief clean up engine
	virtual
	void
	reset();

	
private:
	//! \brief matches the fuzzy match
	bool 
	_match (		ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality fq,					//!< phonetic quality for matching
					const char* phrase,						//!< input phrase
					byte_allocator& all,					//!< external allocator for output container
					byte_allocator& tmp,					//!< external temporary allocator
					candidates_container_t& candidates		//!< [out] output candidate container
					) const;

	//! \brief finds the ngrams that partially intersect
	void 
	partial_intersect(ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality fq,					//!< phonetic quality for matching
					const ngram_key& key,					//!< input sorted n-gram key
					const ngram_key& key_origin,			//!< input original n-gram key
					double score,							//!< match score
					size_t popularity,						//!< n-gram popularity
					size_t word_penalty,					//!< word mismatch penalty
					byte_allocator& all,					//!< external allocator for output container
					byte_allocator& tmp,					//!< external temporary allocator
					candidates_container_t& candidate		//!< [out] output candidate container
					) const;

	//! \brief reconstructs the string from ngram keys
	word_key 
	reconstruct_string(const ngram_key& key,				//!< input ngram key
					byte_allocator& tmp,					//!< external temporary allocator
					bool preserve_case						//!< preserve word case
					) const;
	
	//! \brief creates a case insensitive word key
	static 
	word_key 
	make_string_lower(const char* str,						//!< input string
					size_t len,								//!< string length
					byte_allocator& tmp						//!< external temporary allocator
					);
	//! \brief creates a case insensitive word key
	static 
	word_key 
	make_string_lower(const char* str,						//!< input string
					size_t len,								//!< string length
					char* buf								//!< external pre-allocated buffer
					);
	//! \brief calculates the cross score
	static 
	double 
	calculate_nkey_score(const ngram_key& key1,				//!< input first key
					const ngram_key& key2					//!< input second key
					);
	//! \brief calculates the total score
	static 
	double 
	calculate_score(double phonetic_score,					//!< phonetic score
					double intersection_score,				//!< n-gram intersection score
					size_t popularity,						//!< words popularity
					size_t penalty							//!< words mismatch penalty
					);
	//! \brief finds and removes the metaphone key from the reflection array
	static 
	ub4_t 
	find_and_remove(const metaphone_key& key,				//!< input metaphone key
					ub1_t* array,							//!< [in,out] reflection array
					ub4_t& len								//!< [in,out] reflection length
					);
private:
	
	byte_repository_factory		_memory_factory;			//!< memory factory
	tokenizer					_tokenizer;					//!< tokenizer
	unique_key_generator		_vocabulary_pk_generator;	//!< work key generator
	unique_key_generator		_ngram_pk_generator;		//!< ngram key generator
	unique_key_generator		_metaphone_pk_generator;	//!< metaphone key generator
	unique_key_generator		_reflection_pk_generator;	//!< reflection key generator
	word_entry_t				_word_vocabulary;			//!< word map
	vpk_word_entry_iter_t		_vpk_word_vocabulary;		//!< reverse word map
	metaphone_entry_t			_metaphone_vocabulary;		//!< metaphone map
	mpk_word_entry_iter_t		_mpk_word_vocabulary;		//!< reverse metaphone map
	ngram_entry_t				_ngram_vocabulary;			//!< ngram map
	npk_ngram_entry_iter_t		_npk_ngram_vocabulary;		//!< reverse ngram map
	reflection_entry_t			_reflection_vocabulary;		//!< reflection map
	rpk_ngram_entry_iter_t		_rpk_ngram_vocabulary;		//!< reverse reflection map
	ngram_key_offset_multimap_t	_ngram_partial_map;			//!< partial ngram offsets map
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif //_terimber_fuzzyimpl_h_
