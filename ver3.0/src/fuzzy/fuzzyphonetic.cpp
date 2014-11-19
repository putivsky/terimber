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

#include "fuzzy/fuzzyphonetic.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/vector.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/*
           *********** BEGIN METAPHONE RULES ***********

 Lawrence Philips' RULES follow:

 The 16 consonant sounds:
                                             |--- ZERO represents "th"
                                             |
      B  X  S  K  J  T  F  H  L  M  N  P  R  0  W  Y

 Exceptions:

   Beginning of word: "ae-", "gn", "kn-", "pn-", "wr-"  ----> drop first letter
                      "Aebersold", "Gnagy", "Knuth", "Pniewski", "Wright"

   Beginning of word: "x"                                ----> change to "s"
                                      as in "Deng Xiaopeng"

   Beginning of word: "wh-"                              ----> change to "w"
                                      as in "Whalen"

 Transformations:

   B ----> B      unless at the end of word after "m", as in "dumb", "McComb"

   C ----> X      (sh) if "-cia-" or "-ch-"
           S      if "-ci-", "-ce-", or "-cy-"
                  SILENT if "-sci-", "-sce-", or "-scy-"
           K      otherwise, including "-sch-"

   D ----> J      if in "-dge-", "-dgy-", or "-dgi-"
           T      otherwise

   F ----> F

   G ---->        SILENT if in "-gh-" and not at end or before a vowel
                            in "-gn" or "-gned"
                            in "-dge-" etc., as in above rule
           J      if before "i", or "e", or "y"; if not, double "gg"
           K      otherwise

   H ---->        SILENT if after vowel and no vowel follows
                         or after "-ch-", "-sh-", "-ph-", "-th-", "-gh-"
           H      otherwise

   J ----> J

   K ---->        SILENT if after "c"
           K      otherwise

   L ----> L

   M ----> M

   N ----> N

   P ----> F      if before "h"
           P      otherwise

   Q ----> K

   R ----> R

   S ----> X      (sh) if before "h" or in "-sio-" or "-sia-"
           S      otherwise

   T ----> X      (sh) if "-tia-" or "-tio-"
           0      (th) if before "h"
                  silent if in "-tch-"
           T      otherwise

   V ----> F

   W ---->        SILENT if not followed by a vowel
           W      if followed by a vowel

   X ----> KS

   Y ---->        SILENT if not followed by a vowel
           Y      if followed by a vowel

   Z ----> S
*/

namespace fuzzyphonetic
{

const ub1_t empty_code = 0xff;
const ub1_t blank_code = 0xfe;

//#define DOUBLEMETAPHONE

// adds sounds to list
class metaphone_keeper
{
public:
	metaphone_keeper(_list< ub1_t >& backet, 
#ifdef DOUBLEMETAPHONE
					_list< ub1_t >& backet2,
#endif
					byte_allocator& all) : _backet(backet), 
#ifdef DOUBLEMETAPHONE
											_backet2(backet),
											_alternative(false),
#endif
											_all(all)
	{
		clear();
	}

	inline void clear()
	{
		_code[0] = _code[1] = _alt[0] = _alt[1] = empty_code;
	}

	inline void metaph_add()
	{
		if (_code[0] != empty_code)
		{
			_backet.push_back(_all, _code[0]);
#ifdef DOUBLEMETAPHONE
			_backet2.push_back(_all, _code[0]);
#endif
		}
		if (_code[1] != empty_code)
		{
			_backet.push_back(_all, _code[1]);
#ifdef DOUBLEMETAPHONE
			_backet2.push_back(_all, _code[1]);
#endif
		}
	}

	inline void metaph_add_alt()
	{
		if (_code[0] != empty_code)
			_backet.push_back(_all, _code[0]);
		if (_code[1] != empty_code)
			_backet.push_back(_all, _code[1]);

#ifdef DOUBLEMETAPHONE
		if (_alt[0] != empty_code)
		{
			_alternative = true;
			if (_alt[0] != blank_code)
			{
				_backet2.push_back(_all, _alt[0]);
				if (_alt[1] != empty_code)
					_backet2.push_back(_all, _alt[1]);
			}
		}
		else if (_code[0] != empty_code && _code[0] != blank_code)
		{
			_backet2.push_back(_all, _code[0]);
			if (_code[1] != empty_code)
				_backet2.push_back(_all, _code[1]);
		}
#endif
	}
	

public:
	_list< ub1_t >&	_backet;
#ifdef DOUBLEMETAPHONE
	bool			_alternative;
	_list< ub1_t >&	_backet2;
#endif
	byte_allocator&	_all;
	ub1_t			_code[2];
	ub1_t			_alt[2];
};

// converts single word to the phonetic key
// memory is allocated on internal memory factory is, external allocator is not provided
metaphone_key 
convert_to_metaphone(const char* word, size_t len, byte_allocator& all)
{
	_list< ub1_t > backet
#ifdef DOUBLEMETAPHONE
		, backet2
#endif
		;
	metaphone_keeper meta(backet
#ifdef DOUBLEMETAPHONE
		, backet2
#endif
		, all);

	metaphone_key ret;

	const char* begin = word;
	register size_t index = 0;
	bool isSG = is_slavo_germanic(word, len);

	const size_t slot_size = 8;
	register char slot[slot_size] = {0}; // slot[4] == current
	register const char* pslot[slot_size] = {0};
	const size_t prev4 = 0, prev3 = 1, prev2 = 2, prev = 3, curr = 4, next = 5, next2 = 6, next3 = 7;
	
	register size_t step = 0;
	slot[curr] = to_upper(*word);
	slot[next] = 1 == len ? 0 : to_upper(*(word + 1));
	pslot[curr] = word;
	pslot[next] = 1 == len ? 0 : word + 1;

	register const char* pend = word + len;

	// analyzes the initial char(s)
	switch (slot[curr])
	{
		case 'P':
		case 'K':
		case 'G':
			// PN, KN, GN -> N
			if (len > 1 && slot[next] == 'N')
			{
				++word, ++index;
				memmove(slot, slot + 1, slot_size - 1);
				memmove(pslot, pslot + 1, (slot_size - 1) * sizeof(char*));
			}
			break;

		case 'A':
			// AE -> E
			if (len > 1 && slot[next] == 'E')
			{
				++word, ++index;
				memmove(slot, slot + 1, slot_size - 1);
				memmove(pslot, pslot + 1, (slot_size - 1) * sizeof(char*));
			}
			break;

		case 'W':
			// WR -> R, WH -> H
			if (len > 1 && slot[next] == 'R')
			{
				++word, ++index;
				memmove(slot, slot + 1, slot_size - 1);
				memmove(pslot, pslot + 1, (slot_size - 1) * sizeof(char*));
			}
			break;

		case 'X':
			meta._code[0] = S_NOTE;
			meta._code[1] = empty_code;
			meta.metaph_add();
			++word, ++index;
			memmove(slot, slot + 1, slot_size - 1);
			memmove(pslot, pslot + 1, (slot_size - 1) * sizeof(char*));
			break;
	} // switch
		
	while (index < len)
	{
		// converts to lower upper case
		slot[curr] = to_upper(*word);
		pslot[curr] = word;
		
		if (index + 3 < len)
		{
			slot[next3] = to_upper(*(word + 3));
			slot[next2] = to_upper(*(word + 2));
			slot[next] = to_upper(*(word + 1));

			pslot[next3] = word + 3;
			pslot[next2] = word + 2;
			pslot[next] = word + 1;
		}
		else if (index + 2 < len)
		{
			slot[next3] = 0;
			slot[next2] = to_upper(*(word + 2));
			slot[next] = to_upper(*(word + 1));

			pslot[next3] = 0;
			pslot[next2] = word + 2;
			pslot[next] = word + 1;
		}
		else if (index + 1 < len)
		{
			slot[next3] = slot[next2] = 0;
			slot[next] = to_upper(*(word + 1));

			pslot[next3] = pslot[next2] = 0;
			pslot[next] = word + 1;
		}

		meta.clear();


		switch (slot[curr])
		{
			case 0: // illegal char
				step = 1;
				break;
			case 'A':
			case 'E':
			case 'I':
			case 'O':
			case 'U':
			case 'Y':

				if (slot[prev] == 0)
				{
					meta._code[0] = A_NOTE;
					meta.metaph_add();
				}

				step = 1;
				break;

			case 'B':
				//  B -> P
				meta._code[0] = P_NOTE;
				meta.metaph_add();

				if (slot[next] == 'B')
					step = 2;
				else
					step = 1;

				break;

			case 'C':
				//various germanic
				if(slot[prev2] != 0 && !is_vowel(slot[prev2]) && slot[prev] == 'A' 
						&& slot[next] == 'H'
						&& (slot[next2] != 'I'
							&& slot[next2] != 'E'
							|| check_sub_string(pslot[prev2], pend, "BACHER", 6)
							|| check_sub_string(pslot[prev2], pend, "MACHER", 6)
							)
						)
				{
					meta._code[0] = K_NOTE;
					meta.metaph_add();
					step = 2;
					break;
				}

				// special case 'caesar'
				if (slot[prev] == 0 && check_sub_string(pslot[next], pend, "AESAR", 5))
				{
					meta._code[0] = S_NOTE;
					meta.metaph_add();
					step = 2;
					break;
				}


				if (slot[next] == 'H')
				{

					// italian 'chianti'
					if (check_sub_string(pslot[next2], pend, "IA", 2))
					{
						meta._code[0] = K_NOTE;
						meta.metaph_add();
						step = 2;
						break;
					}

					//finds 'michael'
					if(slot[prev] != 0 && check_sub_string(pslot[next2], pend, "AE", 2))
					{
						meta._code[0] = K_NOTE;
						meta._alt[0] = X_NOTE;
						meta.metaph_add_alt();
						step = 2;
						break;
					}


					//greek roots e.g. 'chemistry', 'chorus'
					if(slot[prev] == 0 && (check_sub_string(pslot[next2], pend, "ARAC", 5) 
									|| check_sub_string(pslot[next2], pend, "ARIS", 5)
									|| check_sub_string(pslot[next2], pend, "OR", 3)
									|| check_sub_string(pslot[next2], pend, "YM", 3)
									|| check_sub_string(pslot[next2], pend, "IA", 3)
									|| check_sub_string(pslot[next2], pend, "EM", 3)
									) 
								&& !check_sub_string(begin, pend, "CHORE", 5))
					{
						meta._code[0] = K_NOTE;
						meta.metaph_add();
						step = 2;
						break;
					}

					// germanic, greek, or otherwise 'ch' for 'kh' sound
					if((check_sub_string(begin, pend, "VAN ", 4)
						|| check_sub_string(begin, pend, "VON ", 4)
						|| check_sub_string(begin, pend, "SCH", 3))
						// 'architect but not 'arch', 'orchestra', 'orchid'
						|| slot[prev2] && check_sub_string(pslot[prev2], pend, "ORCHES", 6)
						|| slot[prev2] && check_sub_string(pslot[prev2], pend, "ARCHIT", 6)
						|| slot[prev2] && check_sub_string(pslot[prev2], pend, "ORCHID", 6)
						|| slot[next2] == 'T' || slot[next2] == 'S'
						|| ((slot[prev] == 'A' || slot[prev] == 'O' || slot[prev] == 'U'|| slot[prev] == 'E'
											|| slot[prev] == 0)
												&& //e.g., 'wachtler', 'wechsler', but not 'tichner'
												(slot[next2] == 'L'
												|| slot[next2] == 'R'
												|| slot[next2] == 'N'
												|| slot[next2] == 'M'
												|| slot[next2] == 'B'
												|| slot[next2] == 'H'
												|| slot[next2] == 'F'
												|| slot[next2] == 'V'
												|| slot[next2] == 'W'
												|| slot[next2] == ' ')
							)
						)
							
					{
						meta._code[0] = K_NOTE;
						meta.metaph_add();
					}
					else
					{
						if (slot[prev] != 0)
						{
							if (check_sub_string(begin, pend, "MC", 2))
							{
							//e.g., "McHugh"
								meta._code[0] = K_NOTE;
								meta.metaph_add();
							}
							else
							{
								meta._code[0] = X_NOTE;
								meta._alt[0] = K_NOTE;
								meta.metaph_add_alt();
							}
						}
						else
						{
							meta._code[0] = X_NOTE;
							meta.metaph_add();
						}
					}
					
					step = 2;
					break;
				} //  CH
				
				//e.g, 'czerny'
				if (slot[next] == 'Z' && !(slot[prev2] == 'W' && slot[prev] == 'I'))
				{
					meta._code[0] = S_NOTE;
					meta._alt[0] = X_NOTE;
					meta.metaph_add_alt();
					step = 2;
					break;
				}

				//e.g., 'focaccia'
				if (slot[next] == 'I' && slot[next2] == 'A')
				{
					meta._code[0] = X_NOTE;
					meta.metaph_add();
					step = 3;
					break;
				}

				//doubles 'C', but not if e.g. 'McClellan'
				if (slot[next] == 'C' && !(slot[prev2] == 0 && slot[prev] != 0 && to_upper(*begin) == 'M'))
				{
					//'bellocchio' but not 'bacchus'
					if ((slot[next2] == 'I'
						|| slot[next2] == 'E'
						|| slot[next2] == 'H')
						&& !check_sub_string(pslot[next2], pend, "HU", 2)
						)
					{
						//'accident', 'accede' 'succeed'
						if ((slot[prev2] == 0 && slot[prev] == 'A')
							|| (slot[prev] == 'U' 
								&& (check_sub_string(pslot[next2], pend, "EE", 2)
									|| check_sub_string(pslot[next2], pend, "ES", 2))
								)
							)
						{
							meta._code[0] = K_NOTE;
							meta._code[1] = S_NOTE;
							meta._alt[0] = K_NOTE;
							meta._alt[1] = S_NOTE;
							meta.metaph_add_alt();
						}
						else
						{
							meta._code[0] = X_NOTE;
							meta.metaph_add();
						}
						step = 3;
						break;
					}
					else
					{
						//Pierce's rule
						meta._code[0] = K_NOTE;
						meta.metaph_add();
						step = 2;
						break;
					}
				}

				if (slot[next] == 'K'
					|| slot[next] == 'G'
					|| slot[next] == 'Q'
					)
				{
					meta._code[0] = K_NOTE;
					meta.metaph_add();
					step = 2;
					break;
				}

				if (slot[next] == 'I'
					|| slot[next] == 'E'
					|| slot[next] == 'Y'
					)
				{
					//italian vs. english
					if (slot[next2] == 'O'
						|| slot[next2] == 'E'
						|| slot[next2] == 'A'
						)
					{
						meta._code[0] = S_NOTE;
						meta._alt[0] = X_NOTE;
						meta.metaph_add_alt();
					}
					else
					{
						meta._code[0] = S_NOTE;
						meta.metaph_add();
					}

					step = 2;
					break;
				}

				meta._code[0] = K_NOTE;
				meta.metaph_add();

				// name sent in 'mac caffrey', 'mac gregor'
				if (slot[next] == ' ' && (slot[next2] == 'C' || slot[next2] == 'Q' || slot[next2] == 'G'))
					step = 3;
				else if ((slot[next] == 'C' || slot[next] == 'Q' || slot[next] == 'G') && !(slot[next] == 'E' || slot[next] == 'I'))
					step = 2;
				else
					step = 1;
				break;

			case 'D':
				if (slot[next] == 'G')
				{
					if (slot[next2] == 'I' 
						|| slot[next2] == 'E'
						|| slot[next2] == 'Y'
						)
					{
						//e.g. 'edge'
						meta._code[0] = J_NOTE;
						meta.metaph_add();
						step = 3;
						break;
					}
					else
					{
						//e.g. 'edgar'
						meta._code[0] = T_NOTE;
						meta._code[1] = K_NOTE;
						meta.metaph_add();
						step = 2;
						break;
					}
				}

				if (slot[next] == 'T' 
					|| slot[next] == 'D'
					)
				{
					meta._code[0] = T_NOTE;
					meta.metaph_add();
					step = 2;
					break;
				}

				meta._code[0] = T_NOTE;
				meta.metaph_add();
				step = 1;
				break;
			case 'F':
				if (slot[next] == 'F')
					step = 2;
				else
					step = 1;
	
				meta._code[0] = F_NOTE;
				meta.metaph_add();
				break;
			case 'G':
				if (slot[next] == 'H')
				{
					if (slot[prev] != 0 && !is_vowel(slot[prev]))
					{
						meta._code[0] = K_NOTE;
						meta.metaph_add();
						step = 2;
						break;
					}

					if (slot[prev3] == 0 && slot[prev] == 0)
					{
						//'ghislane', 'ghiradelli'
						if(slot[next2] == 'I')
						{
							meta._code[0] = J_NOTE;
							meta.metaph_add();
						}
						else
						{
							meta._code[0] = K_NOTE;
							meta.metaph_add();
						}
						
						step = 2;
						break;
					}

					//Parker's rule (with some further refinements) - e.g., 'hugh'
					if (slot[prev2] == 'B'
						|| slot[prev2] == 'H'
						|| slot[prev2] == 'D'
						 	//e.g., 'bough'
						|| slot[prev3] == 'B'
						|| slot[prev3] == 'H'
						|| slot[prev3] == 'D'
						 	//e.g., 'broughton'
						|| slot[prev4] == 'B'
						|| slot[prev4] == 'H'
						)
					{
						step = 2;
						break;
					}
					else
					{
						//e.g., 'laugh', 'McLaughlin', 'cough', 'gough', 'rough', 'tough'
						if (slot[prev2] != 0 
							&& slot[prev] == 'U'
							&& (slot[prev3] == 'C'
								|| slot[prev3] == 'G'
								|| slot[prev3] == 'L'
								|| slot[prev3] == 'R'
								|| slot[prev3] == 'T')
							)
						{
							meta._code[0] = F_NOTE;
							meta.metaph_add();
						}
						else if (slot[prev] != 0 && slot[prev] != 'I')
						{
							meta._code[0] = K_NOTE;
							meta.metaph_add();
						}

						step = 2;
						break;
					}
				}

     			if (slot[next] == 'N')
				{
					if (slot[prev] != 0 && slot[prev2] == 0 && is_vowel(*begin) && !isSG)
					{
						meta._code[0] = K_NOTE;
						meta._code[1] = N_NOTE;
						meta._alt[0] = N_NOTE;
						meta.metaph_add_alt();
					}
					else
					{
						//not e.g. 'cagney'
						if (!check_sub_string(pslot[next2], pend, "EY", 2) 
							&& slot[prev] != 'Y'
							&& !isSG)
						{
							meta._code[0] = N_NOTE;
							meta._alt[0] = K_NOTE;
							meta._alt[1] = N_NOTE;
							meta.metaph_add_alt();
						}
						else
						{
							meta._code[0] = K_NOTE;
							meta._code[1] = N_NOTE;
							meta.metaph_add();
						}
					}
						
					step = 2;
					break;
				}

				//'tagliaro'
				if (slot[next] == 'L' && slot[next2] == 'I' && !isSG)
				{
					meta._code[0] = K_NOTE;
					meta._code[1] = L_NOTE;
					meta._alt[0] = L_NOTE;
					meta.metaph_add_alt();
					step = 2;
					break;
				}

				//-ges-,-gep-,-gel-, -gie- at beginning
				if (slot[prev] == 0
					&& (slot[next] == 'Y'
						|| (slot[next] == 'E' 
							&& (slot[next2] == 'S'
								|| slot[next2] == 'P'
								|| slot[next2] == 'B'
								|| slot[next2] == 'L'
								|| slot[next2] == 'Y'
								|| slot[next2] == 'I'
								|| slot[next2] == 'R'
								)
							)
						|| (slot[next] == 'I' 
							&& (slot[next2] == 'B'
								|| slot[next2] == 'L'
								|| slot[next2] == 'N'
								|| slot[next2] == 'E'
								)							
							)
						)
					)
				{
					meta._code[0] = K_NOTE;
					meta._alt[0] = J_NOTE;
					meta.metaph_add_alt();
					step = 2;
					break;
				}

				// -ger-,  -gy-
				if ((slot[next] == 'E' && slot[next2] == 'R'
					|| slot[next] == 'Y'
					)
					&& !(check_sub_string(begin, pend, "DANGER", 6)
						|| check_sub_string(begin, pend, "RANGER", 6)
						|| check_sub_string(begin, pend, "MANGER", 6))
					&& !(slot[prev] == 'E'
						|| slot[prev] == 'I')
					&& !((slot[prev] == 'R' || slot[prev] == 'O') && slot[prev2] == 'G' && slot[prev3] == 'Y')
					)
				{
					meta._code[0] = K_NOTE;
					meta._alt[0] = J_NOTE;
					meta.metaph_add_alt();
					step = 2;
					break;
				}

				// italian e.g, 'biaggi'
				if ((slot[next] == 'E'
					|| slot[next] == 'I'
					|| slot[next] == 'Y')
					|| ((slot[prev] == 'A' || slot[prev] == 'O') && slot[next] == 'G' && slot[next2] == 'I')
					)
				{
					//obvious germanic
					if ((check_sub_string(begin, pend, "VAN ", 4)
						|| check_sub_string(begin, pend, "VON ", 4)
						|| check_sub_string(begin, pend, "SCH", 3)
						)
						|| (slot[next] == 'E' && slot[next2] == 'T')
						)
					{
						meta._code[0] = K_NOTE;
						meta.metaph_add();
					}
					else
					{
						//always soft if french ending
						if (slot[next] == 'I' && slot[next2] == 'E' && slot[next3] == 'R')
						{
							meta._code[0] = J_NOTE;
							meta.metaph_add();
						}
						else
						{
							meta._code[0] = J_NOTE;
							meta._alt[0] = K_NOTE;
							meta.metaph_add_alt();
						}
					}
					
					step = 2;
					break;
				}

				if (slot[next] == 'G')
					step = 2;
				else
					step = 1;

				meta._code[0] = K_NOTE;
				meta.metaph_add();
				break;

			case 'H':
				//only keep if first & before vowel or btw. 2 vowels
				if ((slot[prev] == 0 || is_vowel(prev))
					&& is_vowel(next)
					)
				{
					meta._code[0] = H_NOTE;
					meta.metaph_add();
					step = 2;
				}
				else //also takes care of 'HH'
					step = 1;
					
				break;

			case 'J':
				//obvious spanish, 'jose', 'san jacinto'
				if ((slot[next] == 'O' && slot[next2] == 'S' && slot[next3] == 'E')
					|| check_sub_string(begin, pend, "SAN J", 5))
				{
					if (index == 0)
					{
						meta._code[0] = H_NOTE;
						meta.metaph_add();
					}
					else
					{
						meta._code[0] = J_NOTE;
						meta._alt[0] = H_NOTE;
						meta.metaph_add_alt();
					}

					step = 1;
					break;
				}
				else
				{
					meta._code[0] = J_NOTE;
					meta._alt[0] = A_NOTE;
					meta.metaph_add_alt();

					//Yankelovich/Jankelowicz
					if(slot[next] == 'J')//it could happen!
					{
						step = 2;
					}
					else
					{
						step = 1;
					}
					break;
				}
				
				if (slot[prev] == 0 && !(slot[next] == 'O' && slot[next2] == 'S' && slot[next3] == 'E'))
				{
					//Yankelovich/Jankelowicz
					meta._code[0] = J_NOTE;
					meta._alt[0] = A_NOTE;
					meta.metaph_add_alt();
				}
				else
				{
					//spanish pron. of e.g. 'bajador'
					if (is_vowel(prev)
						&& !isSG
						&& (slot[next] == 'A'
							|| slot[next] == 'O')
						)
					{
						meta._code[0] = J_NOTE;
						meta._alt[0] = H_NOTE;
						meta.metaph_add_alt();
					}
					else if (index + 1 == len)
					{
						meta._code[0] = J_NOTE;
						meta._alt[0] = blank_code;
						meta.metaph_add_alt();
					}
					else if (!(slot[next] == 'L'
								|| slot[next] == 'T'
								|| slot[next] == 'K'
								|| slot[next] == 'S'
								|| slot[next] == 'N'
								|| slot[next] == 'M'
								|| slot[next] == 'B'
								|| slot[next] == 'Z'
								)
							&& !(slot[prev] == 'S'
								|| slot[prev] == 'K'
								|| slot[prev] == 'L'
								)
								)
					{
						meta._code[0] = J_NOTE;
						meta.metaph_add();
					}
				}

				if (slot[next] == 'J')//it could happen!
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				break;
			case 'K':
				if (slot[next] == 'K')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = K_NOTE;
				meta.metaph_add();
				break;

            case 'L':
                if (slot[next] == 'L')
                {
                    //spanish e.g. 'cabrillo', 'gallegos'
                    if ((index == len - 3 
						&& ((slot[prev] == 'I' && slot[next] == 'L' && (slot[next2] == 'O' || slot[next2] == 'A'))
							|| (slot[prev] == 'A' && slot[next] == 'L' && slot[next2] == 'E'))
						)
                        || ((check_sub_string(begin + len - 2, pend, "AS", 2)
							|| check_sub_string(begin + len - 2, pend, "OS", 2)
							|| to_upper(*(begin + len - 1)) == 'A'
							|| to_upper(*(begin + len - 1)) == 'O')
							&& slot[prev] && (slot[prev] == 'A' && slot[next] == 'L' && slot[next2] == 'E'))
						)
                    {
						meta._code[0] = L_NOTE;
						meta._alt[0] = blank_code;
						meta.metaph_add_alt();
						step = 2;
						break;
                    }

					step = 2;
                }
				else
					step = 1;
					

				meta._code[0] = L_NOTE;
				meta.metaph_add();
                break;

            case 'M':
                if ((slot[prev] == 'U' && slot[next] == 'B') 
                    && ((index + 1 == len) || check_sub_string(pslot[next2], pend,  "ER", 2))
                                //'dumb','thumb'
					|| slot[next] == 'M'
					)
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = M_NOTE;
				meta.metaph_add();
                break;

            case 'N':
                if (slot[next] == 'N')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = N_NOTE;
				meta.metaph_add();
                break;
            case 'P':
                if (slot[next] == 'H')
                {
					meta._code[0] = F_NOTE;
					meta.metaph_add();
					step = 2;
					break;
                }

                //also account for "campbell", "raspberry"
                if (slot[next] == 'P'
					|| slot[next] == 'B')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = P_NOTE;
				meta.metaph_add();
                break;

            case 'Q':
                if (slot[next] == 'Q')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = K_NOTE;
				meta.metaph_add();
                break;

            case 'R':
                //french e.g. 'rogier', but exclude 'hochmeier'
                if(index + 1 == len
					&& !isSG
					&& (slot[prev2] == 'I' && slot[prev] == 'E') 
					&& !(slot[prev4] == 'M' && slot[prev3] == 'E')
						|| (slot[prev4] == 'M' && slot[prev3] == 'A')
					)
				{
					meta._alt[0] = R_NOTE;
					meta.metaph_add_alt();
				}
                else
				{
					meta._code[0] = R_NOTE;
					meta.metaph_add();
				}

                if (slot[next] == 'R')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

                break;

            case 'S':
				
				//special cases 'island', 'isle', 'carlisle', 'carlysle'
				if ((slot[prev] == 'I' || slot[prev] == 'Y') && slot[next] == 'L')
				{
					step = 1;
					break;
				}

				//special case 'sugar-'
				if (slot[prev] == 0 && check_sub_string(pslot[next], pend, "UGAR", 4))
				{
					meta._code[0] = X_NOTE;
					meta._alt[0] = S_NOTE;
					meta.metaph_add_alt();
					step = 1;
					break;
				}

				if (slot[next] == 'H')
				{
					//germanic
					if (check_sub_string(pslot[next2], pend, "EIM", 3)
						|| check_sub_string(pslot[next2], pend, "OEK", 3)
						|| check_sub_string(pslot[next2], pend, "OLM", 3)
						|| check_sub_string(pslot[next2], pend, "OLZ", 3)
						)
					{
						meta._code[0] = S_NOTE;
						meta.metaph_add();
					}
					else
					{
						meta._code[0] = X_NOTE;
						meta.metaph_add();
					}

					step = 2;
					break;
				}

				//italian & armenian
				if (slot[next] == 'I' && (slot[next2] == 'O' || slot[next2] == 'A' && (slot[next3] == 0 || slot[next3] == 'N')))
				{
					if (!isSG)
					{
						meta._code[0] = S_NOTE;
						meta._alt[0] = X_NOTE;
						meta.metaph_add_alt();
					}
					else
					{
						meta._code[0] = S_NOTE;
						meta.metaph_add();
					}

					step = 3;
					break;
				}

				//german & anglicisations, e.g. 'smith' match 'schmidt', 'snider' match 'schneider'
				//also, -sz- in slavic language altho in hungarian it is pronounced 's'
				if ((slot[prev] == 0
					&& (slot[next] == 'M'
						|| slot[next] == 'N'
						|| slot[next] == 'L'
						|| slot[next] == 'W'
						)
					)
					|| (slot[next] == 'Z')
					)
				{
					meta._code[0] = S_NOTE;
					meta._alt[0] = X_NOTE;
					meta.metaph_add_alt();

					if (slot[next] == 'Z')
					{
						step = 2;
					}
					else
					{
						step = 1;
					}

					break;
				}

				if (slot[next] == 'C')
				{
					//Schlesinger's rule
					if (slot[next2] == 'H')
					{
						//dutch origin, e.g. 'school', 'schooner'
						if (check_sub_string(pslot[next3], pend, "OO", 2)
							|| check_sub_string(pslot[next3], pend, "ER", 2)
							|| check_sub_string(pslot[next3], pend, "EN", 2)
							|| check_sub_string(pslot[next3], pend, "UY", 2)
							|| check_sub_string(pslot[next3], pend, "ED", 2)
							|| check_sub_string(pslot[next3], pend, "EM", 2)
							)
						{
							//'schermerhorn', 'schenker'
							if (check_sub_string(pslot[next3], pend, "ER", 2)
								|| check_sub_string(pslot[next3], pend, "EN", 2)
								)
							{
								meta._code[0] = X_NOTE;
								meta._alt[0] = S_NOTE;
								meta._alt[1] = K_NOTE;
								meta.metaph_add_alt();
							}
							else
							{
								meta._code[0] = S_NOTE;
								meta._code[1] = K_NOTE;
								meta.metaph_add();
							}

							step = 3;
							break;
						}
						else
						{
							if (slot[prev] == 0 && !is_vowel(*(begin + 3)) && len > 3 && to_upper(*(begin + 3)) != 'W')
							{
								meta._code[0] = X_NOTE;
								meta._alt[0] = S_NOTE;
								meta.metaph_add_alt();
							}
							else
							{
								meta._code[0] = X_NOTE;
								meta.metaph_add();
							}

							step = 3;
							break;
						}
					}

					if (slot[next2] == 'I'
						|| slot[next2] == 'E'
						|| slot[next2] == 'Y'
						)
					{
						meta._code[0] = S_NOTE;
						meta.metaph_add();
						step = 3;
						break;
					}
	
					meta._code[0] = S_NOTE;
					meta._code[1] = K_NOTE;
					meta.metaph_add();
					step = 3;
					break;
				}

				//french e.g. 'resnais', 'artois'
				if (index + 1 == len && (((slot[prev2] == 'A' || slot[prev2] == 'O') && slot[prev] == 'I')))
				{
					meta._alt[0] = S_NOTE;
					meta.metaph_add_alt();
				}
				else
				{
					meta._code[0] = S_NOTE;
					meta.metaph_add();
				}

				if (slot[next] == 'S'
					|| slot[next] == 'Z')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}
				break;

            case 'T':
                if (slot[next] == 'I' && slot[next2] == 'O' && slot[next3] == 'N')
                {
					meta._code[0] = X_NOTE;
					meta.metaph_add();
					step = 3;
					break;
                }

                if (slot[next] == 'I' && slot[next2] == 'A'
					|| slot[next] == 'C' && slot[next2] == 'H')
                {
					meta._code[0] = X_NOTE;
					meta.metaph_add();
					step = 3;
					break;
                }

                if (slot[next] == 'H'
					|| slot[next] == 'T' && slot[next2] == 'H')
                {
                    //special case 'thomas', 'thames' or germanic
                    if (check_sub_string(pslot[next2], pend, "OM", 2)
						|| check_sub_string(pslot[next2], pend, "AM", 2)
						|| check_sub_string(begin, pend, "VAN ", 4)
						|| check_sub_string(begin, pend, "VON ", 4)
						|| check_sub_string(begin, pend, "SCH", 3)
						)
                    {
						meta._code[0] = T_NOTE;
						meta.metaph_add();
                    }
					else
					{
						meta._code[0] = TH_NOTE;
						meta._alt[0] = T_NOTE;
						meta.metaph_add_alt();
                    }

					step = 2;
					break;
                }

                if (slot[next] == 'T'
					|| slot[next] == 'D')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = T_NOTE;
				meta.metaph_add();
                break;

            case 'V':
                if (slot[next] == 'V')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

				meta._code[0] = F_NOTE;
				meta.metaph_add();
                break;

			case 'W':
				//can also be in the middle of the word
				if (slot[next] == 'R')
				{
					meta._code[0] = R_NOTE;
					meta.metaph_add();
					step = 2;
					break;
				}

				if (slot[prev] == 0
					&& (is_vowel(next) || slot[next] == 'H'))
				{
					//Wasserman should match Vasserman
					if (is_vowel(next))
					{
						meta._code[0] = A_NOTE;
						meta._alt[0] = F_NOTE;
						meta.metaph_add_alt();
					}
					else
					{
						//need Uomo to match Womo
						meta._code[0] = A_NOTE;
						meta.metaph_add();
					}
				}

				//Arnow should match Arnoff
				if ((index + 1 == len && is_vowel(prev))
					|| (slot[prev] == 'E' && slot[next] == 'S' && slot[next2] == 'K' && slot[next3] == 'I')
					|| (slot[prev] == 'O' && slot[next] == 'S' && slot[next2] == 'K' && slot[next3] == 'I')
					|| (slot[prev] == 'O' && slot[next] == 'S' && slot[next2] == 'K' && slot[next3] == 'Y')
					|| check_sub_string(begin, pend, "SCH", 3)
					)
				{
					meta._code[0] = empty_code;
					meta._code[1] = empty_code;
					meta._alt[0] = F_NOTE;
					meta._alt[1] = empty_code;
					meta.metaph_add_alt();
					step = 1;
					break;
				}

				//polish e.g. 'filipowicz'
				if (slot[next] == 'I' && (slot[next2] == 'C' || slot[next2] == 'T') && slot[next3] == 'Z')
				{
					meta._code[0] = T_NOTE;
					meta._code[1] = S_NOTE;
					meta._alt[0] = F_NOTE;
					meta._alt[1] = X_NOTE;
					meta.metaph_add_alt();
					step = 4;
					break;
				}

				//else skips it
				step = 1;
				break;

            case 'X':
                //french e.g. breaux
                if (!(index + 1 == len 
					&& (slot[prev3] == 'I' && slot[prev2] == 'A' && slot[prev] == 'U')
						|| (slot[prev3] == 'E' && slot[prev2] == 'A' && slot[prev] == 'U')
						|| (slot[prev2] == 'A' && slot[prev] == 'U')
						|| (slot[prev2] == 'O' && slot[prev] == 'U')
						)
					)
				{
					meta._code[0] = K_NOTE;
					meta._code[1] = S_NOTE;
					meta.metaph_add();
				}

                if (slot[next] == 'C'
					|| slot[next] == 'X')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}

                break;

            case 'Z':
                //chinese pinyin e.g. 'zhao'
                if (slot[next] == 'H')
                {
					meta._code[0] = J_NOTE;
					meta.metaph_add();
					step = 2;
                    break;
                }
				else
				{
                    if ((slot[next] == 'Z' && (slot[next2] == 'O' || slot[next2] == 'I' || slot[next2] == 'A'))
						|| (isSG && slot[prev] != 0 && slot[prev] != 'T')
						)
                    {
						meta._code[0] = S_NOTE;
						meta._alt[0] = T_NOTE;
						meta._alt[1] = S_NOTE;
						meta.metaph_add_alt();
                    }
                    else
					{
						meta._code[0] = S_NOTE;
						meta.metaph_add();
					}
				}

                if (slot[next] == 'Z')
				{
					step = 2;
				}
				else
				{
					step = 1;
				}
                break;

            default:
				step = 1;
				break;
		} // switch


		// how many we skipped
		assert(step > 0 && step < 7);
		memmove(slot, slot + step, slot_size - step);
		memmove(pslot, pslot + step, (slot_size - step) * sizeof(char*));
		word += step, index += step;
	}

	// allocates memory
	ret._length = (ub4_t)backet.size();

	if (ret._length)
	{
		ret._array = (ub1_t*)all.allocate(ret._length);
		// copies memory
		index = 0;

		for (_list< ub1_t >::const_iterator it = backet.begin(); it != backet.end(); ++it, ++index)
		{
			// converts phonet to code
			//assert(*it < metaphone_reflection::REFSIZE);
			ret._array[index] = *it;
		}
	}

	return ret;
}


void 
convert_to_reflection(const char* word, size_t len, byte_allocator& all, reflection_key& reflection) // output reflection
{
	char* buf = (char*)all.allocate(len + 1);

	size_t offset = 0, shift = 0;
	register char ch;

	while (shift < len)
	{
		switch (ch = to_upper(word[shift]))
		{
		case 0:
			break;
		default:
			buf[offset++] = ch;
		}

		++shift;
	}


	buf[offset] = 0;

	metaphone_key phonet = convert_to_metaphone(buf, offset, all);

	for (size_t i = 0; i < phonet._length; ++i)
	{
		assert(phonet._array[i] < reflection_key::REFSIZE); 

		if (reflection._array[phonet._array[i]] < reflection_key::REFMAX)
			++reflection._array[phonet._array[i]];
	}
}


}


#pragma pack()
END_TERIMBER_NAMESPACE
