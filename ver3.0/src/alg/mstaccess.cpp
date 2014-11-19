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

#include "alg/mstaccess.h"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/vector.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/list.hpp"

#include "alg/mst.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class cluster_engine_impl
class cluster_engine_impl : public cluster_engine
{
public:
	//! \brief constructor
	cluster_engine_impl() : 
		_client(0) 
	{
	}

	//! \brief does the real clustering process
	virtual 
	void 
	do_clustering(	const cluster_client* client,			//!< client interface
					double max_vertex_distance,				//!< max vertex distance
					double max_cluster_distance,			//!< max cluster distance
					double avg_cluster_distance				//!< average cluster distance
					)
	{
		_clusters.clear();
		_all.clear_all();

		if (!client)
			return;

		_client = client;

		cluster_processor< cluster_engine_impl, cluster_engine_impl > 
		processor(*this, *this, max_vertex_distance, max_cluster_distance, avg_cluster_distance);
		const cluster_map_t& clusters = processor.get_clusters();

		// copies results
		for (cluster_map_t::const_iterator iter = clusters.begin(); iter != clusters.end(); ++iter)
		{
			cluster_list_t dummy;
			cluster_map_t::iterator it = _clusters.insert(_all, iter.key(), dummy).first;
			for (cluster_list_t::const_iterator liter = iter->begin(); liter != iter->end(); ++liter)
			{
				it->push_back(_all, *liter);
			}
		}
	}

	//! \brief returns the count of clusters
	virtual 
	size_t 
	get_clusters_count() const
	{
		return _clusters.size();
	}

	//! \brief gets the cluster size
	virtual 
	size_t 
	get_cluster_size(size_t cluster_index					//!< cluster index
					) const
	{
		if (cluster_index < 0 || cluster_index >= _clusters.size())
			return os_minus_one;


		cluster_map_t::const_iterator iter = _clusters.find(cluster_index);

		return iter->size();

	}

	//! \brief gets the item by cluster index and object index
	virtual 
	size_t 
	get_cluster_object(size_t cluster_index,				//!< cluster index
					size_t object_index						//!< object index
					) const
	{
		if (cluster_index < 0 || cluster_index >= _clusters.size())
			return os_minus_one;


		cluster_map_t::const_iterator iter = _clusters.find(cluster_index);

		if (object_index < 0 || object_index >= iter->size())
			return os_minus_one;

		size_t i = 0;
		for (cluster_list_t::const_iterator liter = iter->begin(); liter != iter->end(); ++liter, ++i)
		{
			if (i == object_index)
				return *liter;
		}

		return os_minus_one;
	}

	//! \brief number of items
	inline 
	size_t 
	size() const 
	{ 
		return _client->size(); 
	}

	//! \brief gets the distance
	inline 
	double 
	distance(		size_t from, 
					size_t to
					) const 
	{ 
		return _client->distance(from, to); 
	}

	//! \brief notify message
	inline 
	void 
	notify(			const char* msg
					) const 
	{ 
		_client->notify(msg); 
	}
private:
	const cluster_client*			_client;				//!< client interface
	cluster_map_t					_clusters;				//!< clusters
	byte_allocator					_all;					//!< external allocator
};


#pragma pack()
END_TERIMBER_NAMESPACE

// class factory for clustering algorithm engine
cluster_engine*
cluster_engine_factory::get_cluster_engine() 
{ 
	return new TERIMBER::cluster_engine_impl(); 
}
