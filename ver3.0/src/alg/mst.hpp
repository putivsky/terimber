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

#ifndef _terimber_mst_hpp_
#define _terimber_mst_hpp_

#include "alg/mst.h"
#include "base/stack.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

template < class T >
inline
priority_queue< T >::priority_queue(byte_allocator& all, size_t N, const _vector< T >& pred, size_t dim) :
	_dim(dim), _Num(0), _pred(pred)
{
	_pq.resize(all, N + 1, 0);
	_qp.resize(all, N + 1, 0);
}

template < class T >
inline 
bool 
priority_queue< T >::empty() const 
{ 
	return !_Num; 
}

template < class T >
inline 
size_t 
priority_queue< T >::size() const 
{ 
	return _Num; 
}

template < class T >
inline 
void
priority_queue< T >::insert(size_t v)
{
	_pq[++_Num] = v;
	_qp[v] = _Num;
	fixUp(_Num);
}

template < class T >
inline 
size_t 
priority_queue< T >::getmin()
{
	exchange(1, _Num);
	fixDown(1, _Num - 1);
	return _pq[_Num--];
}

template < class T >
inline 
void
priority_queue< T >::lower(size_t k)
{
	fixUp(_qp[k]);
}

template < class T >
inline 
void
priority_queue< T >::exchange(size_t i, size_t j)
{
	size_t t = _pq[i];
	_pq[i] = _pq[j];
	_pq[j] = t;

	_qp[_pq[i]] = i;
	_qp[_pq[j]] = j;
}

template < class T >
inline 
void
priority_queue< T >::fixUp(size_t i)
{
	size_t j;
	while (i > 1 && _pred[_pq[j = (i + _dim - 2) / _dim]] > _pred[_pq[i]])
	{
		exchange(i, j);
		i = j;
	}
}

template < class T >
inline 
void
priority_queue< T >::fixDown(size_t k, size_t N)
{
	size_t j;
	while ((j = _dim * (k - 1) + 2) <= N)
	{
		for (size_t i = j + 1; i < j + _dim && i <= N; ++i)
		{
			if (_pred[_pq[j]] > _pred[_pq[i]])
				j = i;
		}

		if (_pred[_pq[k]] <= _pred[_pq[j]])
			break;

		exchange(k, j);
		k = j;
	}
}

/////////////////////////////////////////////////////////////////////////	

template < class T, class N >
mst< T, N >::mst(const T& container, const N& notifier, byte_allocator& all, byte_allocator& temp) :
	_container(container), _notifier(notifier), _length(container.size())
{
	_wt.resize(all, _length, -1.0);
	mst_edge info(-1, -1, -1.0);
	_fr.resize(all, _length, info);
	_mst.resize(all, _length, info);

	for (size_t v = 0; v < _length; ++v)
	{
		if (_mst[v]._distance == -1.0)
		{
			pfs(v, temp);
		}
	}
}


template < class T, class N >
inline 
const mst_vec_t& 
mst< T, N >::get_mst() const 
{ 
	return _mst; 
}

template < class T, class N >
inline 
void
mst< T, N >::pfs(size_t s, byte_allocator& tmp)
{
	tmp.reset();

	priority_queue< double > pQ(tmp, _length, _wt);
	pQ.insert(s);

	size_t step = 0;
	char buf[128];

	while (!pQ.empty())
	{
		size_t v = pQ.getmin();
		_mst[v] = _fr[v];

		for (size_t w = 0; w < _length; ++w)
		{
			if (v == w)
				continue;

			double distance;

			if (_fr[w]._distance == -1.0)
			{
				distance = _container.distance(v, w);

				_wt[w] = distance;

				pQ.insert(w);

				_fr[w]._from = v;
				_fr[w]._to = w;
				_fr[w]._distance = distance;
			}
			else if (_mst[w]._distance == -1.0
					&& (distance = _container.distance(v, w)) < _wt[w])
			{
				_wt[w] = distance;

				pQ.lower(w);

				_fr[w]._from = v;
				_fr[w]._to = w;
				_fr[w]._distance = distance;
			}
		} // for

		if (++step && !(step % 100))
		{
			str_template::strprint(buf, 128, "edges processed %d", step);
			_notifier.notify(buf);
		}
	}
}

////////////////////////////////////////////////////////

template < class T, class N >
cluster_processor< T, N >::cluster_processor(const T& container, const N& notifier, double max_vertex_distance, double max_cluster_distance, double avg_cluster_distance) :
	_container(container), _notifier(notifier)
{
	cut(max_vertex_distance, max_cluster_distance, avg_cluster_distance);
}

template < class T, class N >
void
cluster_processor< T, N >::cut(double max_vertex_distance, double max_cluster_distance, double avg_cluster_distance)
{
	byte_allocator temp, lookup;

	char buf[128];

	str_template::strprint(buf, 128, "mst process has been started...");
	_notifier.notify(buf);

	// builds mst
	mst< T, N > obj(_container, _notifier, temp, lookup);

	// gets mst edges
	const mst_vec_t& edges = obj.get_mst();

	str_template::strprint(buf, 128, "mst edges to be clustered: %d", edges.size());
	_notifier.notify(buf);
	
	// fills out map and priority map
	mst_head_map_t priority_map;

	for (mst_vec_t::const_iterator iter = edges.begin(); iter != edges.end(); ++iter)
	{
		cluster_info cinfo(-1, 0.0);
		_mst_map.insert(temp, *iter, cinfo);
		
		mst_head_map_t::iterator piter = priority_map.find(iter->_from);

		if (piter != priority_map.end())
			++*piter;
		else
			priority_map.insert(temp, iter->_from, 1);
	}

	_vector< mst_head_map_citer_t > priority_vector;
	// resizes
	priority_vector.resize(temp, priority_map.size(), priority_map.end());
	// copies
	size_t pindex = 0;
	mst_head_map_t::const_iterator piter = priority_map.begin();

	for (; piter !=  priority_map.end(); ++piter, ++pindex)
		priority_vector[pindex] = piter;

	mst_head_compare_counts sorter;
	std::sort(priority_vector.begin(), priority_vector.end(), sorter);

	//  defines cluster ident
	size_t step = 0;
	size_t ident = os_minus_one;
	cluster_map_t::iterator citer;
	cluster_mst_map_t::iterator miter;

	// iterator for container
	for (_vector< mst_head_map_citer_t >::const_iterator viter = priority_vector.begin(); viter != priority_vector.end(); ++viter)
	{
		mst_edge item(viter->key(), os_minus_one, 0.0);

		cluster_mst_map_t::iterator fiter = _mst_map.lower_bound(item);

		if (fiter == _mst_map.end()
			|| fiter.key()._from != viter->key())
			continue;

		size_t cluster_ident = fiter->_cluster_ident;
		
		if (cluster_ident == os_minus_one)
		{
			// adds new cluster
			cluster_ident = ++ident;
			_list< size_t > info;
			citer = _clusters.insert(_all, cluster_ident, info).first;
			
			citer->push_back(_all, fiter.key()._from);
			// assigns correct cluster ident
			fiter->_cluster_ident = cluster_ident;
		}

		lookup.reset();
		// looks up neighbors
		_stack< size_t > walkup_stack;
		walkup_stack.push(lookup, fiter.key()._from);

		do
		{
			size_t i = walkup_stack.top();
			walkup_stack.pop();

			mst_edge item(i, os_minus_one, 0.0);

			miter = _mst_map.lower_bound(item);

			size_t from;

			while (miter != _mst_map.end()
				&& (from = miter.key()._from) == i)
			{
				const mst_edge& r = miter.key();

				if (r._distance <= max_vertex_distance // not far away
					&& miter->_cluster_distance <= max_cluster_distance // not a long way
					&& (fiter.key()._to == r._to || miter->_cluster_ident == os_minus_one)
					)
				{

					if (fiter.key()._to != r._to)
					{
						miter->_cluster_ident = cluster_ident;
						miter->_cluster_distance = fiter->_cluster_distance + r._distance;
						citer->push_back(_all, r._to);
					}

					// adds to stack
					walkup_stack.push(lookup, r._to);
				} //if

				++miter;
			} // while
		}
		while (!walkup_stack.empty());

		if (++step && !(step % 100))
		{
			str_template::strprint(buf, 128, "objects processed %d", step);
			_notifier.notify(buf);
		}
	} // for

	str_template::strprint(buf, 128, "processing orphans...");
	_notifier.notify(buf);

	// gets orphans
	size_t oindex = 0;
	size_t orphans = 0;

	for (miter = _mst_map.begin(); miter != _mst_map.end(); ++miter, ++oindex)
	{
		size_t cluster_ident = miter->_cluster_ident;
		
		if (cluster_ident != os_minus_one)
			continue;

		size_t cindex = 0;
		double min_agg_distance = 1.0;

		// for clusters
		for (cluster_map_t::iterator it = _clusters.begin(); it != _clusters.end(); ++it, ++cindex)
		{
			double davg = avg_distance(it, oindex);

			if (davg < min_agg_distance)
			{
				cluster_ident = cindex;
				min_agg_distance = davg;
				citer = it;
			}
		}


		if (cluster_ident != -1 && min_agg_distance <= avg_cluster_distance)
		{
			miter->_cluster_ident = cluster_ident;
			miter->_cluster_distance = min_agg_distance;
			citer->push_back(_all, oindex);
		}
		else
		{
			// adds new cluster
			cluster_ident = ++ident;
			cluster_list_t info;
			cluster_map_t::iterator citer = _clusters.insert(_all, cluster_ident, info).first;
			citer->push_back(_all, oindex);
		}

		if (++orphans && !(orphans % 100))
		{
			str_template::strprint(buf, 128, "orphans processed %d", orphans);
			_notifier.notify(buf);
		}
	}

	str_template::strprint(buf, 128, "found orphans: %d", orphans);
	_notifier.notify(buf);
}

template < class T, class N >
inline
const cluster_map_t&
cluster_processor< T, N >::get_clusters() const 
{ 
	return _clusters; 
}

template < class T, class N >
inline
double
cluster_processor< T, N >::avg_distance(cluster_map_t::const_iterator citer, size_t index) const
{
	double ret = 0.0;
	size_t count = 0;
	for (cluster_list_t::const_iterator iter = citer->begin();
											iter != citer->end();
											++iter, ++count)
	{
		ret += _container.distance(*iter, index);
	}

	return ret / count;
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
