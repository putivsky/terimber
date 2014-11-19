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

#ifndef _terimber_mst_h_
#define _terimber_mst_h_

#include "base/vector.h"

BEGIN_TERIMBER_NAMESPACE

#pragma pack(4)

//! \typedef priority_queue_vec_t
//! vector of idents
typedef _vector< size_t >	priority_queue_vec_t;

//! \class priority_queue
//! \brief priority queue for items
template < class T >
class priority_queue
{
public:
	//! \brief constructor
	inline 
	priority_queue(	byte_allocator& all,					//!< external allocator
					size_t N,								//!< queue size
					const _vector< T >& pred,				//!< predicate
					size_t dim = 3							//!< dimension
					);
	//! \brief checks if queue is empty
	inline 
	bool 
	empty() const;
	//! \brief size of queue
	inline 
	size_t 
	size() const;
	//! \brief inserts item
	inline 
	void 
	insert(			size_t v								//!< item
					);
	//! \brief gets min element
	inline 
	size_t 
	getmin();
	//! \brief puts item at the lower position
	inline 
	void 
	lower(			size_t v								//!< item
					);

private:
	//! \brief interchanges items at positions i, j
	inline 
	void 
	exchange(		size_t i,								//!< first index
					size_t j								//!< second index
					);
	//! \brief moves up
	inline 
	void 
	fixUp(			size_t i								//!< index
					);
	//! \brief moves down
	inline 
	void 
	fixDown(		size_t k,								//!< index
					size_t N								//!< size
					);
	
private:
	size_t					_dim;							//!< dimension
	size_t					_Num;							//!< queue size
	priority_queue_vec_t	_pq;							//!< queue vector
	priority_queue_vec_t	_qp;							//!< reverse queue vector
	const _vector< T >&		_pred;							//!< predicate
};

//! \class mst_edge
//! \brief MST edge
class mst_edge
{
public:
	//! \brief constructor
	mst_edge(		size_t from,							//!< from ident
					size_t to,								//!< to ident
					double distance) :						//!< distance
		_from(from), 
		_to(to), 
		_distance(distance) 
	{
	}
	//! \brief operator less
	inline 
	bool 
	operator<(const mst_edge& x) const
	{ 
		return _from != x._from ? _from < x._from : _to < x._to; 
	}

	size_t		_from;										//!< from ident
	size_t		_to;										//!< to ident
	double		_distance;									//!< distance
};


//! keeps head population for future sorting
//! \typedef mst_head_map_t
typedef _map< size_t, size_t >				mst_head_map_t;
//! \typedef mst_head_map_citer_t
//! \brief mst_head_map_t iterator
typedef mst_head_map_t::const_iterator		mst_head_map_citer_t;

//! \class mst_head_compare_counts
//! \brief predicate
class mst_head_compare_counts
{
public:
	//! \brief operator less
	inline 
	bool 
	operator()(		mst_head_map_citer_t x,					//!< first iterator	
					mst_head_map_citer_t y					//!< second iterator
					) const
	{ 
		return *x > *y; 
	}
};

//! \class cluster_info
//! \brief cluster info
class cluster_info
{
public:
	//! \brief constructor
	cluster_info(	size_t ident,							//!< cluster ident
					double distance							//!< distance
					) : 
		_cluster_ident(ident), 
		_cluster_distance(distance) 
	{
	}

	size_t	_cluster_ident;									//!< cluster ident
	double	_cluster_distance;								//!< cluster distance
};

//! \typedef mst_vec_t
//! \brief vector of MST edges
typedef _vector< mst_edge > mst_vec_t;
//! \typedef mst_dist_t
//! \brief vector of distances
typedef _vector< double >	mst_dist_t;

//! \class mst
//! \brief minimum spanning tree
template < class T, class N >
class mst
{
public:
	//! \brief constructor
	mst(			const T& container,						//!< container of items
					const N& notifier,						//!< notifier class
					byte_allocator& all,					//!< external data allocator
					byte_allocator& temp					//!< external temporary allocator
					);
	//! \brief returns mst vector
	inline 
	const mst_vec_t& 
	get_mst() const;

private:
	//! \brief function actually construct MST
	void pfs(		size_t s,								//!< size
					byte_allocator& tmp						//!< temporary allocator
					);

private:
	const T&			_container;							//!< container
	const N&			_notifier;							//!< notifier
	const size_t		_length;							//!< container length
	mst_dist_t			_wt;								//!< array of distances
	mst_vec_t			_fr;								//!< array of edges
	mst_vec_t			_mst;								//!< mst
};

//! \typedef cluster_list_t
//! \brief list of items in cluster
typedef _list< size_t >					cluster_list_t;
//! \typedef cluster_map_t
//! \brief maps cluster ident to list of items
typedef _map< size_t, cluster_list_t >	cluster_map_t;
//! \typedef cluster_mst_map_t
//! \brief maps edge to cluster info
typedef _map< mst_edge, cluster_info >	cluster_mst_map_t;

//! \class cluster_processor
//! \brief cluster processor
template < class T, class N >
class cluster_processor
{
public:
	//! \brief constructor
	cluster_processor(const T& container,					//!< container
					const N& notifier,						//!< notifier
					double max_vertex_distance,				//!< max vertex distance
					double max_cluster_distance,			//!< max cluster distance
					double avg_cluster_distance				//!< average cluster distance
					);
	//! \brief returns cluster map
	inline 
	const 
	cluster_map_t& 
	get_clusters() const;

private:
	//! \brief cuts the mst according to distances
	void 
	cut(			double max_vertex_distance,				//!< max vertex distance
					double max_cluster_distance,			//!< max cluster distance
					double avg_cluster_distance				//!< average cluster distance
					);
	//! \brief calculates average distance
	inline 
	double 
	avg_distance(	cluster_map_t::const_iterator citer,	//!< cluster map iterator
					size_t index							//!< item index
					) const;

private:
	const T&						_container;				//!< container
	const N&						_notifier;				//!< notifier
	cluster_mst_map_t				_mst_map;				//!< mst
	cluster_map_t					_clusters;				//!< cluster map
	byte_allocator					_all;					//!< data allocator
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
