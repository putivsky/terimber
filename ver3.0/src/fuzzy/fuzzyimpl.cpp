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

#include "fuzzy/fuzzyimpl.h"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/common.hpp"
#include "base/map.hpp"
#include "base/vector.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/date.h"

#include "smart/byterep.hpp"

#include "fuzzy/fuzzyphonetic.hpp"

fuzzy_matcher*
fuzzy_matcher_factory::get_fuzzy_matcher(size_t memory_usage)
{
	return new TERIMBER::fuzzy_matcher_impl(memory_usage);
}

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
double wrapper_find_metaphone_distance(const metaphone_key& key, metaphone_entry_citer_t y, byte_allocator& tmp, size_t max_penalty)
{
	return (double)fuzzyphonetic::find_metaphone_distance(key, y.key(), tmp, max_penalty);
}


inline 
double wrapper_find_reflection_distance(const reflection_key& key, reflection_entry_citer_t y, byte_allocator& tmp, size_t max_penalty)
{
	return (double)fuzzyphonetic::find_reflection_distance(key, y.key(), tmp, max_penalty);
}


template < class C, class P, class Func  >
class lookup_distance
{
public:
	inline
	lookup_distance(bool extended, size_t max_distance, const C& container, const P& key, Func f, byte_allocator& tmp) : 
		_extended(extended),
		_max_distance(max_distance),
		_container(container), 
		_key(key), 
		_tmp(tmp),
		_f(f), 
		_stage(0),
		_range(container.equal_range(key)),
		_save_range(_range)
	{
		if (_save_range.first != _container.end())
			--_save_range.first;
	}

	inline
	pair< TYPENAME C::const_iterator, double > next()
	{
		pair< TYPENAME C::const_iterator, double > ret(_container.end(), 0.0);

		if (_stage == 0)
		{
			if (_range.first != _range.second)
			{
				ret.first = _range.first;
				++_range.first;
				return ret;
			}

			_stage = 1;
		}

		if (_stage == 1 && _extended)
		{
			if (_save_range.first != _container.end())
			{
				// compares distance
				_tmp.reset();
				ret.second = _f(_key, _save_range.first, _tmp, _max_distance + 1);
				if (ret.second <= _max_distance)
				{
					ret.first = _save_range.first;
					--_save_range.first;
					return ret;
				} // if
			} // if

			_stage = 2;
		}

		if (_stage == 2 && _extended)
		{
			if (_save_range.second != _container.end())
			{
				// compares distance
				_tmp.reset();
				ret.second = _f(_key, _save_range.second, _tmp, _max_distance + 1);
				if (ret.second <= _max_distance)
				{
					ret.first = _save_range.second;
					++_save_range.second;
					return ret;
				}
			}

			_stage = 3;
		}

		return ret;
	}


private:
	const C&	_container;
	const P&	_key;
	Func		_f;
	byte_allocator& _tmp;
	bool		_extended;
	size_t		_max_distance;

	size_t				_stage;
	typename C::paircc_t _range;
	typename C::paircc_t _save_range;
};
/////////////////////////////////////////////////////////////////////

fuzzy_matcher_impl::fuzzy_matcher_impl(size_t memory_usage)
{
	assert(_memory_factory.is_ready());
}

// virtual
fuzzy_matcher_impl::~fuzzy_matcher_impl()
{
}

//! \brief clean up engine
//virtual
void
fuzzy_matcher_impl::reset()
{
	_vocabulary_pk_generator.clear();
	_ngram_pk_generator.clear();
	_metaphone_pk_generator.clear();
	_reflection_pk_generator.clear();


	_tokenizer.clear_regex();
	_tokenizer.clear_abbr();

	_word_vocabulary.clear();
	_vpk_word_vocabulary.clear();
	_metaphone_vocabulary.clear();
	_mpk_word_vocabulary.clear();
	_ngram_vocabulary.clear();
	_npk_ngram_vocabulary.clear();
	_reflection_vocabulary.clear();
	_rpk_ngram_vocabulary.clear();
	_ngram_partial_map.clear();

	_memory_factory->reset();
}

// adds a new n-gram to the internal repository (utf-8)
// caller can add the same n-gram many times
// class will support internal references count
// virtual 
size_t 
fuzzy_matcher_impl::add(const char* phrase, byte_allocator& all)
{
	size_t npk = 0;
	if (!phrase)
		return npk;

	tokenizer_output_sequence_t tokens;
	_tokenizer.tokenize(phrase, tokens, all, 0);

	// 1. Looks up dictionaries for all alphabetic, digits, symbols, etc - all recognizable token types
	_list< size_t > idents;
	size_t offset = 0;
	size_t count = 0;

	for (tokenizer_output_sequence_t::const_iterator it = tokens.begin(); count < MAX_PHRASE_TOKENS && it != tokens.end(); offset += it->_len, ++it)
	{
		switch (it->_type)
		{
			case TT_ALPHABETIC:
			case TT_DIGIT:
				break; // considers only alphbetics or digits
			default:
				continue;
		}

		++count;

		word_key wkey(phrase + offset, __min(it->_len, MAX_TOKEN_LENGTH));

		// tries to find
		word_entry_t::iterator it_word = _word_vocabulary.find(wkey);
		if (it_word == _word_vocabulary.end())
		{
			// inserts a new one

			// checks metaphone
			metaphone_key mkey = fuzzyphonetic::convert_to_metaphone(wkey._str, wkey._len, all);
			metaphone_entry_t::iterator miter = _metaphone_vocabulary.find(mkey);
			
			if (miter != _metaphone_vocabulary.end()) // found metaphone
			{
				++miter->_ref;
			}
			else
			{
				// new metaphone
				size_t mpk = _metaphone_pk_generator.generate();
				// makes a copy
				metaphone_key mkey_new;
				mkey_new._array = (ub1_t*)_memory_factory->allocate(mkey_new._length = mkey._length);
				memcpy(mkey_new._array, mkey._array, mkey_new._length);

				// inserts a new metaphone key.entry
				metaphone_entry mentry(mpk);
				miter = _metaphone_vocabulary.insert(mkey_new, mentry).first;
			}

			// makes a copy
			char* wptr = _memory_factory->allocate(wkey._len + 1);
			memcpy(wptr, wkey._str, wkey._len);
			wptr[wkey._len] = 0;
			
			// generates new vpk
			size_t vpk = _vocabulary_pk_generator.generate();

			word_key wkey(wptr, wkey._len);
			word_entry wentry(vpk, miter);
			// inserts new word key/entry
			it_word = _word_vocabulary.insert(wkey, wentry).first;

			// inserts vpk into reverse map
			_vpk_word_vocabulary.insert(it_word->_vpk, it_word);
			// inserts mpk into metaphone reverse multimap
			_mpk_word_vocabulary.insert(miter->_mpk, it_word);
		}
		else // word is already here
		{
			// increment reference
			++it_word->_ref;
		}

		idents.push_back(all, it_word->_vpk);
	}

	//  working with ngrams
	size_t ngram_length = idents.size();

	if (ngram_length)
	{
		// makes a temp copy
		ngram_key nkey, okey;
		okey._length = nkey._length = ngram_length;
		size_t index = 0;
		nkey._array = (size_t*)all.allocate(ngram_length * sizeof(size_t));
		okey._array = (size_t*)all.allocate(ngram_length * sizeof(size_t));
		for (_list< size_t >::const_iterator it_ident = idents.begin(); it_ident != idents.end(); ++it_ident, ++index)
			okey._array[index] = nkey._array[index] = *it_ident;

		// sorts idents to avoid dependence from
		std::sort(nkey._array, nkey._array + nkey._length);

		ngram_entry_t::iterator it_ngram = _ngram_vocabulary.find(nkey);
		 
		if (it_ngram == _ngram_vocabulary.end())
		{
			// checks reflection
			size_t phrase_len = strlen(phrase);
			reflection_key rkey;
			fuzzyphonetic::convert_to_reflection(phrase, phrase_len, all, rkey);

			reflection_entry_t::iterator riter = _reflection_vocabulary.find(rkey);
			if (riter != _reflection_vocabulary.end()) // found reflection
			{
				++riter->_ref;
			}
			else
			{
				// inserts new reflection
				size_t rpk = _reflection_pk_generator.generate();
				reflection_entry rentry(rpk);
				riter = _reflection_vocabulary.insert(rkey, rentry).first;
			}

			npk = _ngram_pk_generator.generate();
			ngram_entry nentry(npk, riter);

			size_t* nptr = (size_t*)_memory_factory->allocate(ngram_length * sizeof(size_t));
			memcpy(nptr, nkey._array, nkey._length * sizeof(size_t));
			size_t* optr = (size_t*)_memory_factory->allocate(ngram_length * sizeof(size_t));
			memcpy(optr, okey._array, okey._length * sizeof(size_t));

			nkey._array = nptr;
			nentry._origin._array = optr;
			nentry._origin._length = okey._length;

			it_ngram = _ngram_vocabulary.insert(nkey, nentry).first;

			// inserts into reverse npk map
			_npk_ngram_vocabulary.insert(it_ngram->_npk, it_ngram);
			// inserts into reverse reflection multimap
			_rpk_ngram_vocabulary.insert(riter->_rpk, it_ngram);

			// checks the length
			if (ngram_length > 1)
			{
				for (size_t offset = 1; offset < ngram_length; ++offset)
				{
					ngram_key_offset pkey(offset, it_ngram);
					_ngram_partial_map.insert(pkey, true);
				}
			}
		}
		else
		{
			npk = it_ngram->_npk;
			++it_ngram->_ref;
		}
	}

	return npk;
}
	
// removes previously added ngram
// caller can remove the same n-gram many times unless the reference count goes to zero
// virtual 
bool 
fuzzy_matcher_impl::remove(const char* phrase, byte_allocator& all)
{
	if (!phrase)
		return false;

	tokenizer_output_sequence_t tokens;
	_tokenizer.tokenize(phrase, tokens, all, 0);

	// 1. Looks up dictionary for all alphabetic, digits, symbols, etc - all recognizable token types
	_list< size_t > idents;
	size_t offset = 0;
	size_t count = 0;

	for (tokenizer_output_sequence_t::const_iterator it = tokens.begin(); count < MAX_PHRASE_TOKENS && it != tokens.end(); offset += it->_len, ++it)
	{
		switch (it->_type)
		{
			case TT_ALPHABETIC:
			case TT_DIGIT:
				break; // considers only alphbetics or digits
			default:
				continue;
		}

		++count;

		word_key wkey(phrase + offset, __min(it->_len, MAX_TOKEN_LENGTH));
		word_entry_t::iterator it_word = _word_vocabulary.find(wkey);

		if (it_word != _word_vocabulary.end())
		{
			if (it_word->_ref == 1)
			{
				// looks up in multimap
				size_t mpk = it_word->_miter->_mpk;
				mpk_word_entry_iter_t::iterator iter_metaphone = _mpk_word_vocabulary.lower_bound(mpk);

				while (mpk == iter_metaphone.key())
				{
					if (*iter_metaphone == it_word)
					{
						_mpk_word_vocabulary.erase(iter_metaphone);
						break;
					}

					++iter_metaphone;
				}

				// memory release
				_memory_factory->deallocate((char*)it_word.key()._str);
				// erases from reverse pk -> iter dictionary
				_vpk_word_vocabulary.erase(it_word->_vpk);
				// returns id to the generator
				_vocabulary_pk_generator.save(it_word->_vpk);

				// checks metaphone
				if (it_word->_miter->_ref == 1)
					_metaphone_vocabulary.erase(it_word->_miter);
				else
					--it_word->_miter->_ref;
				// erases from dictionary
				_word_vocabulary.erase(it_word);
			}
			else
				--it_word->_ref;
		}

		idents.push_back(all, it_word->_vpk);
	}

	//  working with ngrams
	size_t ngram_length = idents.size();

	if (ngram_length)
	{
		// makes a temp copy
		ngram_key nkey;
		nkey._length = ngram_length;
		size_t index = 0;
		nkey._array = (size_t*)all.allocate(ngram_length * sizeof(size_t));
		for (_list< size_t >::const_iterator it_ident = idents.begin(); it_ident != idents.end(); ++it_ident, ++index)
			nkey._array[index] = *it_ident;

		std::sort(nkey._array, nkey._array + nkey._length);

		ngram_entry_t::iterator it_ngram = _ngram_vocabulary.find(nkey);
		 
		if (it_ngram != _ngram_vocabulary.end())
		{
			if (it_ngram->_ref == 1)
			{
				size_t rpk = it_ngram->_riter->_rpk;
				rpk_ngram_entry_iter_t::iterator iter_reflaction = _rpk_ngram_vocabulary.lower_bound(rpk);

				while (rpk == iter_reflaction.key())
				{
					if (*iter_reflaction == it_ngram)
					{
						_rpk_ngram_vocabulary.erase(iter_reflaction);
						break;
					}

					++iter_reflaction;
				}


				// checks the length
				if (ngram_length > 1)
				{
					for (size_t offset = 1; offset < ngram_length; ++offset)
					{
						ngram_key_offset pkey(offset, it_ngram);
						ngram_key_offset_multimap_t::iterator iter_lower = _ngram_partial_map.lower_bound(pkey);
						ngram_key_offset_multimap_t::iterator iter_upper = _ngram_partial_map.upper_bound(pkey);

						// checks all iterators - we are dealing with multimap
						while (iter_lower != iter_upper)
						{
							if (iter_lower.key()._iter == it_ngram)
								iter_lower = _ngram_partial_map.erase(iter_lower);
							else
								++iter_lower;
						}
					}
				}

				// memory release
				_memory_factory->deallocate((char*)it_ngram->_origin._array);
				_memory_factory->deallocate((char*)it_ngram.key()._array);

				// erases from reverse pk -> iter dictionary
				_npk_ngram_vocabulary.erase(it_ngram->_npk);
				_ngram_pk_generator.save(it_ngram->_npk);

				// checks metaphone
				if (it_ngram->_riter->_ref == 1)
					_reflection_vocabulary.erase(it_ngram->_riter);
				else
					--it_ngram->_riter->_ref;

				_ngram_vocabulary.erase(it_ngram);
			}
			else
				--it_ngram->_ref;
		}
	}

	return true;
}

// removes previously added ngram
// caller can remove the same n-gram many times unless the reference count goes to zero
// virtual 
bool fuzzy_matcher_impl::remove(size_t ident, 
					byte_allocator& all)
{
	npk_ngram_entry_iter_t::iterator it_ident = _npk_ngram_vocabulary.find(ident);

	if (it_ident == _npk_ngram_vocabulary.end())
		return false;

	ngram_entry_iter_t it_ngram = *it_ident;
	size_t ngram_length = it_ngram.key()._length;

	if (it_ngram->_ref == 1)
	{
		size_t rpk = it_ngram->_riter->_rpk;
		rpk_ngram_entry_iter_t::iterator iter_reflaction = _rpk_ngram_vocabulary.lower_bound(rpk);

		while (rpk == iter_reflaction.key())
		{
			if (*iter_reflaction == it_ngram)
			{
				_rpk_ngram_vocabulary.erase(iter_reflaction);
				break;
			}

			++iter_reflaction;
		}


		// checks the length
		if (ngram_length > 1)
		{
			for (size_t offset = 1; offset < ngram_length; ++offset)
			{
				ngram_key_offset pkey(offset, it_ngram);
				ngram_key_offset_multimap_t::iterator iter_lower = _ngram_partial_map.lower_bound(pkey);
				ngram_key_offset_multimap_t::iterator iter_upper = _ngram_partial_map.upper_bound(pkey);

				// checks all iterators - we are dealing with multimap
				while (iter_lower != iter_upper)
				{
					if (iter_lower.key()._iter == it_ngram)
						iter_lower = _ngram_partial_map.erase(iter_lower);
					else
						++iter_lower;
				}
			}
		}

		// memory release
		_memory_factory->deallocate((char*)it_ngram->_origin._array);
		_memory_factory->deallocate((char*)it_ngram.key()._array);

		// erases from reverse pk -> iter dictionary
		_npk_ngram_vocabulary.erase(it_ngram->_npk);
		_ngram_pk_generator.save(it_ngram->_npk);

		// checks metaphone
		if (it_ngram->_riter->_ref == 1)
			_reflection_vocabulary.erase(it_ngram->_riter);
		else
			--it_ngram->_riter->_ref;

		_ngram_vocabulary.erase(it_ngram);
	}
	else
		--it_ngram->_ref;

	return true;
}

// does the fuzzy match
// virtual 
bool 
fuzzy_matcher_impl::match(	ngram_quality nq,
					phonetic_quality fq,
					const char* phrase, 
					byte_allocator& all, 
					byte_allocator& tmp,
					_list< const char* >& suggestions) const
{
	candidates_container_t candidates;

	if (!_match(nq, fq, phrase, all, tmp, candidates))
		return false;

	// sorts candidates
	tmp.reset();
	vector_container_citer_t vec; 
	candidate_sorter sorter(candidates, vec, tmp);

	// converts all candidates back to words
	for (vector_container_citer_t::const_iterator iter_candidate = vec.begin(); iter_candidate != vec.end(); ++iter_candidate)
	{
		// finds word and 
		const ngram_key& ckey = (*iter_candidate).key();

		// finds the ngram original order
		ngram_entry_t::const_iterator iter_ngram = _ngram_vocabulary.find(ckey);
		assert(iter_ngram != _ngram_vocabulary.end());

		const ngram_key& key_origin = iter_ngram->_origin;

		word_key wkey = reconstruct_string(key_origin, all, true);
		suggestions.push_back(all, wkey._str);
	}


	return true;
}

// virtual 
bool 
fuzzy_matcher_impl::match(	ngram_quality nq,
						phonetic_quality fq,
						const char* phrase, 
						byte_allocator& all, 
						byte_allocator& tmp,
						_list< size_t >& suggestions) const
{
	candidates_container_t candidates;

	if (!_match(nq, fq, phrase, all, tmp, candidates))
		return false;

	tmp.reset();
	vector_container_citer_t vec;
	candidate_sorter sorter(candidates, vec, tmp);

	// converts all candidates to their idents
	for (vector_container_citer_t::const_iterator iter_candidate = vec.begin(); iter_candidate != vec.end(); ++iter_candidate)
	{
		// finds key 
		const ngram_key& ckey = (*iter_candidate).key();
		// gets ngram pk
		ngram_entry_t::const_iterator iter_pk = _ngram_vocabulary.find(ckey);
		assert(iter_pk != _ngram_vocabulary.end());

		suggestions.push_back(all, iter_pk->_npk);
	}

	return true;
}

bool 
fuzzy_matcher_impl::_match (ngram_quality nq,
					phonetic_quality fq,
					const char* phrase, 
					byte_allocator& all, 
					byte_allocator& tmp,
					candidates_container_t& candidates) const
{
	if (!phrase)
		return false;

	size_t max_phonet_distance = (fq == pq_high ? 0 : (fq == pq_normal ? 1 : 2));
	size_t max_word_distance = (fq == pq_high ? 1 : (fq == pq_normal ? 2 : 4));

	// 1. tokenizes first
	tokenizer_output_sequence_t tokens;
	_tokenizer.tokenize(phrase, tokens, tmp, 0);

	size_t phrase_length = strlen(phrase);

	_list< string_desc > words;
	_list< metaphone_key > metaphones;

	// 2. converts phrase to metaphone
	metaphone_key phrase_key = fuzzyphonetic::convert_to_metaphone(phrase, phrase_length, all);

	// 3. finds reflection
	reflection_key phrase_reflection;
	fuzzyphonetic::convert_to_reflection(phrase, phrase_length, all, phrase_reflection);

	size_t offset = 0;
	size_t count = 0;
	
	// looks through all tokens
	for (tokenizer_output_sequence_t::const_iterator it = tokens.begin(); count < MAX_PHRASE_TOKENS && it != tokens.end(); offset += it->_len, ++it)
	{
		switch (it->_type)
		{
			case TT_ALPHABETIC:
			case TT_DIGIT:
				break; // considers only alphbetics or digits
			default:
				continue;
		}

		++count;
		
		// makes a local copy of the token and converts it to the lower case
		size_t len = __min(it->_len, MAX_TOKEN_LENGTH);
		char* ptr = (char*)all.allocate(len + 1);
		for (size_t i = 0; i < len; ++i)
			ptr[i] = fuzzyphonetic::to_lower(*(phrase + offset + i));

		ptr[len] = 0;

		string_desc desc;
		desc._vpk = 0;
		desc._str = ptr;
		desc._len = len;

		// stores each token in a word array
		words.push_back(all, desc);

		// converts token to metaphone
		metaphone_key key = fuzzyphonetic::convert_to_metaphone(ptr, len, all);
		metaphones.push_back(all, key);
	}

	// we only have garbage
	if (count == 0) // nothing to check
		return false;


	// 1. searches for whole phrase as one token (phonet match, score 0,1,2 depends on phonet quality) -> getting ngram idents
	// what possible user cases can we cover?
	// 1a. user entered two or more words without whitespaces; PhilCollins
	// 2. then tokenizes phrase and does search for each token; repeats the step above -> gets vocabulary pks
	// 2a. for the set of vocabulary pks -> constructs sorted ngram key(s)
	// 2b. searches for partial intersection (depends on ngram quality score 0, 1, 2)

	word_key phraselow = make_string_lower(phrase, phrase_length, all);

	// looks for metaphone match
	lookup_distance< metaphone_entry_t, metaphone_key, double (*)(const metaphone_key&, metaphone_entry_citer_t, byte_allocator&, size_t) >
		lookup_metaphone(true, max_phonet_distance, _metaphone_vocabulary, phrase_key, wrapper_find_metaphone_distance, tmp);

	pair< metaphone_entry_t::const_iterator, double > iter_metaphone;
	while ((iter_metaphone = lookup_metaphone.next()).first != _metaphone_vocabulary.end())
	{
		// finds vpk by mpk
		size_t mpk = iter_metaphone.first->_mpk;
		// metaphone score
		double score = iter_metaphone.second;
		
		mpk_word_entry_iter_t::const_iterator iter_word = _mpk_word_vocabulary.lower_bound(mpk);

		while (iter_word != _mpk_word_vocabulary.end()
			&& iter_word.key() == mpk
			)
		{
			ngram_key nkey;
			size_t vpk = (*iter_word)->_vpk;
			nkey._array = &vpk;
			nkey._length = 1;
			// single word popularity
			size_t popularity = (*iter_word)->_ref;

			word_key wkey = make_string_lower((*iter_word).key()._str, (*iter_word).key()._len, tmp);
			// word matching penalty
			size_t word_penalty = fuzzyphonetic::find_word_distance(phraselow._str, phraselow._len, wkey._str, wkey._len, tmp, max_phonet_distance + 1);

			if (word_penalty <= max_word_distance)
				partial_intersect(nq, fq, nkey, nkey, score, popularity, word_penalty + 3*(count - 1), all, tmp, candidates);
			++iter_word;
		}
	}


	if (count > 1) // ngrams
	{
		_list< string_desc >::const_iterator wit = words.begin();
		_list< metaphone_key >::const_iterator mit = metaphones.begin();
		_list< sorted_ngram_suggestions_t > score_sugs;
		size_t items = 0;

		for (; wit != words.end(); ++wit, ++mit)
		{
			sorted_ngram_suggestions_t score_dummy;
			sorted_ngram_suggestions_t& score_ref = *score_sugs.push_back(all, score_dummy);

			// looks for each token
			// looks for metaphone match
			lookup_distance< metaphone_entry_t, metaphone_key, double (*)(const metaphone_key&, metaphone_entry_citer_t, byte_allocator&, size_t) >
				lookup_metaphone(nq == nq_high ? false : true, max_phonet_distance, _metaphone_vocabulary, *mit, wrapper_find_metaphone_distance, tmp);

			pair< metaphone_entry_t::const_iterator, double > iter_metaphone;
			while ((iter_metaphone = lookup_metaphone.next()).first != _metaphone_vocabulary.end())
			{
				// finds vpk by mpk
				size_t mpk = iter_metaphone.first->_mpk;
				double score = iter_metaphone.second;
				
				mpk_word_entry_iter_t::const_iterator iter_word = _mpk_word_vocabulary.lower_bound(mpk);

				while (iter_word != _mpk_word_vocabulary.end()
					&& iter_word.key() == mpk
					)
				{
					size_t vpk = (*iter_word)->_vpk;
					size_t popularity = (*iter_word)->_ref;

					word_key wkey = make_string_lower((*iter_word).key()._str, (*iter_word).key()._len, tmp);
					// word matching penalty
					size_t word_penalty = fuzzyphonetic::find_word_distance(wit->_str, wit->_len, wkey._str, wkey._len, tmp, max_phonet_distance + 1);

					if (word_penalty <= max_word_distance)
					{
						ngram_suggestions_info score_p(vpk, popularity, score, word_penalty);
						score_ref.insert(all, calculate_score(score, 1.0, popularity, word_penalty), score_p);
					}

					++iter_word;
				} // while
			} // while

			if (!score_ref.empty())
				++items;
			else
			{
				score_sugs.pop_back();
			}
		} // for

		// works only if there is something
		if (items)
		{
			// fills out iterators
			ngram_suggestions_iter_list_t iter_list;
			
			_list< sorted_ngram_suggestions_t >::iterator iter_score = score_sugs.begin();

			size_t av_per_token = (size_t)pow(100.0, 1./items);

			if (av_per_token == 0)
				av_per_token = 1;

			for (; iter_score != score_sugs.end(); ++iter_score)
			{
				while (iter_score->size() > av_per_token) // reduce
				{
					iter_score->erase(--(iter_score->end()));
				}

				ngram_suggestions_iter_info info(iter_score->begin(), iter_score->end());
				iter_list.push_back(all, info);
			}

			ngram_key scorekey, scorekey_origin;
			scorekey_origin._length = scorekey._length = items;
			scorekey._array = (size_t*)all.allocate(scorekey._length * sizeof(size_t));
			scorekey_origin._array = (size_t*)all.allocate(scorekey_origin._length * sizeof(size_t));

			ngram_suggestions_iter_list_t::iterator fiter = iter_list.begin();

			while (true)
			{
				// here we can navigate through all suggestions in a row.
				double score_score = 0.0;
				size_t score_pop = 0;
				size_t score_penalty = 0;
				size_t index = 0;

				for (ngram_suggestions_iter_list_t::iterator liter = iter_list.begin(); liter != iter_list.end(); ++liter, ++index)
				{
					score_score += liter->_current->_score;
					score_pop += liter->_current->_popularity;
					score_penalty += liter->_current->_penalty;

					scorekey_origin._array[index] = scorekey._array[index] = liter->_current->_vpk;
				}

				std::sort(scorekey._array, scorekey._array + index);

				// gets all possible combinations
				partial_intersect(nq, fq, scorekey, scorekey_origin, score_score / items, score_pop / items, score_penalty / items, all, tmp, candidates);

				while (fiter != iter_list.end())
				{
					// saves iter
					sorted_ngram_suggestions_citer_t citer = fiter->_current;
					// moves forward if suggestions are still available
					if (citer != fiter->_end
						&& ++citer != fiter->_end)
					{
						++(fiter->_current); // moves to the next suggestion

						// resets prev iter, if any
						if (fiter != iter_list.begin())
						{
							ngram_suggestions_iter_list_t::iterator startiter = iter_list.begin();

							do
							{
								startiter->_current = startiter->_begin;
								++startiter;
							}
							while (startiter != fiter);

							// resets begin iter
							fiter = iter_list.begin();
						} // if

						break;
					} // if

					++fiter;
				} // while

				if (fiter == iter_list.end())
					break;
			} // while 
		} // items
	}

	// looks for reflection
	lookup_distance< reflection_entry_t, reflection_key, double (*)(const reflection_key&, reflection_entry_citer_t, byte_allocator&, size_t) >
		lookup_reflection(true, max_phonet_distance, _reflection_vocabulary, phrase_reflection, wrapper_find_reflection_distance, tmp);

	pair< reflection_entry_t::const_iterator, double > iter_reflection;
	while ((iter_reflection = lookup_reflection.next()).first != _reflection_vocabulary.end())
	{
		// finds npk by rpk
		size_t rpk = iter_reflection.first->_rpk;
		double score = iter_reflection.second;

		rpk_ngram_entry_iter_t::const_iterator iter_ngram = _rpk_ngram_vocabulary.lower_bound(rpk);

		while (iter_ngram != _rpk_ngram_vocabulary.end()
			&& iter_ngram.key() == rpk
			)
		{
			const ngram_key& nkey_sorted = (*iter_ngram).key();

			// adds to candidates
			if (candidates.end() == candidates.find(nkey_sorted))
			{
				const ngram_key& nkey_origin = (*iter_ngram)->_origin;
				size_t popularity = (*iter_ngram)->_ref;
				
				// word matching penalty if it is a single word
				if (nkey_origin._length == 1)
				{
					word_key reckey = reconstruct_string(nkey_origin, tmp, false);
					size_t word_penalty = fuzzyphonetic::find_word_distance(phraselow._str, phraselow._len, reckey._str, reckey._len, tmp, max_phonet_distance + 1);
					
					if (word_penalty <= max_word_distance)
					{
						candidate_info info(calculate_score(score, count - 1.0, popularity, word_penalty+ 3*(count - 1)), score, popularity, word_penalty); 
						candidates.insert(all, nkey_sorted, info);
					}
				}
				else // ngram
				{
					// makes a copy of the phrase key
					char word[MAX_TOKEN_LENGTH]; // space for lower case string
					ub1_t array[MAX_TOKEN_LENGTH]; // binary array - metaphoneic codes
					ub4_t length = __min((ub4_t)MAX_TOKEN_LENGTH, phrase_key._length); 
					memcpy(array, phrase_key._array, length);
					memset(array + length, 0xff, MAX_TOKEN_LENGTH - length);

					size_t word_penalty = 0;
					size_t phone_penalty = 0;
					// tries to match phonets
					for (size_t p = 0; p < nkey_origin._length; ++p)
					{
						// finds phonet
						vpk_word_entry_iter_t::const_iterator it_p = _vpk_word_vocabulary.find(nkey_origin._array[p]);
						assert(it_p != _vpk_word_vocabulary.end());

						const metaphone_key& r_key = (*it_p)->_miter.key();

						phone_penalty += r_key._length - find_and_remove(r_key, array, length);

						word_key wkey = make_string_lower((*it_p).key()._str, (*it_p).key()._len, word);
						word_penalty += ((strstr(phraselow._str, wkey._str) != 0) ? 0 : 3);
					} // for


					//phone_penalty += length;

					
					if (phone_penalty <= max_phonet_distance
						&& word_penalty <= max_word_distance)
					{
						size_t count_diff = __max(nkey_origin._length, count) - __min(nkey_origin._length, count);
						candidate_info info(calculate_score(score, (double)count_diff, popularity, phone_penalty+ 3*word_penalty), score, popularity, word_penalty); 
						candidates.insert(all, nkey_sorted, info);
					}
				}
			}
			++iter_ngram;
		}
	}

	return true;
}

void 
fuzzy_matcher_impl::partial_intersect(ngram_quality nq,
						phonetic_quality fq,
						const ngram_key& key,
						const ngram_key& key_origin,
						double score,
						size_t popularity,
						size_t word_penalty,
						byte_allocator& all, 
						byte_allocator& tmp,
						candidates_container_t& candidates) const
{
	ngram_key mkey;
	mkey._length = 1;
	mkey._array = 0;

	double nq_score = (nq == nq_high ? 0.1 : (nq == nq_normal ? 0.26 : 0.34));

	// looks for beginning
	for (size_t m = 0; m < key._length; ++m)
	{
		mkey._array = &key._array[m];

		// searches in the main map
		ngram_entry_t::const_iterator iter_main = _ngram_vocabulary.lower_bound(mkey);
		while (iter_main != _ngram_vocabulary.end()
			&& mkey._array[0] == iter_main.key()._array[0]
			)
		{
			// TO DO - get candidate score
			size_t pop = iter_main->_ref;
			const ngram_key& nkey = iter_main.key();
			const ngram_key& nkey_origin = iter_main->_origin;

			double interscore = calculate_nkey_score(nkey, key);
			if (interscore < nq_score 
				&& candidates.end() == candidates.find(nkey)
				)
			{
				size_t len_diff = __max(nkey._length, key._length) - __min(nkey._length, key._length);
				//len_diff /= __max(nkey._length, key._length); 
				double interscore_origin = calculate_nkey_score(nkey_origin, key_origin);
				double final_interscore = (interscore + interscore_origin) / 2.0;

				candidate_info info(calculate_score(score, final_interscore, (popularity + pop) / 2, word_penalty + 3*len_diff), (score + final_interscore) / 2.0, (popularity + pop) / 2, word_penalty); 
				candidates.insert(all, nkey, info);
			}

			++iter_main;
		}
	}

	if (!_ngram_partial_map.empty())
	{
		// finds the longest offset
		ngram_key_offset_multimap_t::const_iterator iter_upper = _ngram_partial_map.end();
		--iter_upper;

		size_t max_tokens = iter_upper.key()._offset;
		
		// reallocates
		mkey._length = max_tokens + 1;
		mkey._array = (size_t*)tmp.allocate(mkey._length * sizeof(size_t));
		memset(mkey._array, 0, mkey._length * sizeof(size_t));
		

		for (size_t l = 0; l < key._length; ++l)
		{
			// looks for offset maps
			for (size_t i = 1; i <= max_tokens; ++i)
			{
				mkey._array[i] = key._array[l];
				mkey._length = i + 1;
				ngram_key_offset pkey(i, &mkey);
				ngram_key_offset_multimap_t::const_iterator it_partial = _ngram_partial_map.lower_bound(pkey);
				while (it_partial != _ngram_partial_map.end()
					&& it_partial.key()._offset == i
					&& it_partial.key()._iter.key()._array[i] == key._array[l]
					)
				{
					// TO DO - get candidate score
					size_t pop = it_partial.key()._iter->_ref;
					const ngram_key& nkey = it_partial.key()._iter.key();
					const ngram_key& nkey_origin = it_partial.key()._iter->_origin;

					double interscore = calculate_nkey_score(nkey, key);

					if (interscore < nq_score 
						&& candidates.end() == candidates.find(nkey)
						)
					{
						size_t len_diff = __max(nkey._length, key._length) - __min(nkey._length, key._length);
						//len_diff /= __max(nkey._length, key._length); 
						double interscore_origin = calculate_nkey_score(nkey_origin, key_origin);
						double final_interscore = (interscore + interscore_origin) / 2.0;
						
						candidate_info info(calculate_score(score, final_interscore, (popularity + pop) / 2, word_penalty + 3*len_diff), (score + final_interscore) / 2.0, (popularity + pop) / 2, word_penalty); 
						candidates.insert(all, nkey, info);
					}

					++it_partial;
				} // while
			} // for
		} // for
	} // if
}


// static 
double 
fuzzy_matcher_impl::calculate_nkey_score(const ngram_key& key1, const ngram_key& key2)
{
	double ret = 0.0;
	size_t len1 = key1._length;
	size_t len2 = key2._length;

	for (size_t i1 = 0, i2 = 0; i1 < len1 && i2 < len2;)
	{
		if (key1._array[i1] < key2._array[i2])
			++i1;
		else if (key2._array[i2] < key1._array[i1])
			++i2;
		else
			++i1, ++i2, ret += 2;
	}

	return (len1 + len2 - ret) / (len1 + len2);
}

word_key
fuzzy_matcher_impl::reconstruct_string(const ngram_key& key, byte_allocator& tmp, bool preserve_case) const
{
	_list< word_key > words;
	size_t total_bytes = 0;

	for (size_t i = 0; i < key._length; ++i)
	{
		vpk_word_entry_iter_t::const_iterator iter_word_pk = _vpk_word_vocabulary.find(key._array[i]);
		assert(iter_word_pk != _vpk_word_vocabulary.end());

		word_key desc((*iter_word_pk).key());
		total_bytes += desc._len + 1;
		words.push_back(tmp, desc);
	}

	char* ret = (char*)tmp.allocate(total_bytes);
	size_t offset = 0;

	for (_list< word_key >::const_iterator it = words.begin(); it != words.end(); ++it)
	{
		if (preserve_case)
		{
			memcpy(ret + offset, it->_str, it->_len);
		}
		else
		{
			for (size_t c = 0; c < it->_len; ++c)
				ret[offset + c] = fuzzyphonetic::to_lower(it->_str[c]);
		}
		offset += it->_len;
		ret[offset++] = ' ';
	}

	ret[total_bytes - 1] = 0;
	word_key r(ret, total_bytes - 1); 
	return r;
}

// static 
word_key 
fuzzy_matcher_impl::make_string_lower(const char* str, size_t len, byte_allocator& tmp)
{
	char* ret = (char*)tmp.allocate(len + 1);
	size_t offset = 0;

	for (size_t c = 0; c < len; ++c)
		ret[offset + c] = fuzzyphonetic::to_lower(str[c]);

	ret[len] = 0;
	word_key r(ret, len); 
	return r;
}

// static 
word_key 
fuzzy_matcher_impl::make_string_lower(const char* str, size_t len, char* buf)
{
	size_t offset = 0;

	for (size_t c = 0; c < len; ++c)
		buf[offset + c] = fuzzyphonetic::to_lower(str[c]);

	buf[len] = 0;
	word_key r(buf, len); 
	return r;
}

// static 
double 
fuzzy_matcher_impl::calculate_score(double phonetic_score, double intersection_score, size_t popularity, size_t penalty)
{
	return (0.75 * phonetic_score + 0.9 * intersection_score + 0.25 * penalty + 1. / popularity);
}

//static 
ub4_t 
fuzzy_matcher_impl::find_and_remove(const metaphone_key& key, ub1_t* array, ub4_t& len)
{
	register ub4_t len_substr = key._length;
	register ub4_t len_str = len;

	if (!len_substr || !len || len_substr > len_str)
		return 0;

	register ub1_t* p = key._array;
	register ub1_t* _this = array;
	register ub1_t* _next = 0;

	// similar to strstr algorithm
	while ((_next = (ub1_t*)memchr(_this, *p, len_str))) // find first entry
	{
		register ub4_t consumed = (ub4_t)((_next - array) + len_substr);
		if (len < consumed)
			return 0;

		 // we have enough space
		if (!memcmp(_next, p, len_substr))
		{
			// removes
			memmove(_next, _next + len_substr, len - ((_next - array) + len_substr));
			len -= len_substr;
			return len_substr;
		}
		else
		{
			_this = _next + 1;
			len_str = (ub4_t)(len - (_this - array));
		}
	}

	return 0;
}

#pragma pack()
END_TERIMBER_NAMESPACE
