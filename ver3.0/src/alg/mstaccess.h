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

#ifndef _terimber_mstaccess_h_
#define _terimber_mstaccess_h_


#include "allinc.h"

//! temporary external wrapper for clustering algorithm demonstration

//! \class cluster_client
//! \brief abstract class for external clients
class cluster_client
{
public:
	//! \brief desrtuctor 
	virtual ~cluster_client() 
	{
	}

	//! \brief returns the size of array of object prepared for clustering
	virtual 
	size_t 
	size() const = 0;
	
	//! \brief returned distance may NOT be negative and should be the same for (from, to) and (to, from)
	//! index is zero based [0, size() - 1]
	//! you can imagine the performance imrovement 
	//! when virtual call will be replaced by inline function for template implementation
	virtual 
	double 
	distance(		size_t from,							//!< from ident
					size_t to								//!< to ident
					) const = 0;	

	//! \brief just for notification what's going on inside the clustering engine
	virtual 
	void 
	notify(			const char* msg							//!< notification message
					) const = 0;
};

//! \class cluster_engine
//! \brief class wrapper around template cluster engine
class cluster_engine
{
public:
	//! \brief destructor
	virtual
	~cluster_engine() {}
	//! \brief do the real clustering process
	//! typically values (0.8, 2.0, 0.95) for soft clustering
	//! typically values (0.5, 0.9, 0.8) for hard clustering
	virtual 
	void 
	do_clustering(	const cluster_client* client, 
					double max_vertex_distance,				//!< max allowed distance between two object in one cluster obj1 -> d1 -> obj2
					double max_cluster_distance,			//!< max allowed aggregated distance in the object chain belonging to the same cluster
					double avg_cluster_distance				//!< for orphans adoption = (d1 + d2 + ... + dn) / n
					) = 0;
	//! \brief returns the count of clusters
	virtual 
	size_t 
	get_clusters_count() const = 0;
	//! \brief returns (if cluster index is in the boundaries) the size of cluster
	virtual 
	size_t 
	get_cluster_size(size_t cluster_index					//!< cluster index
					) const = 0;
	//! \brief returns (if cluster index is in the boundaries) the intem by index
	virtual 
	size_t 
	get_cluster_object(size_t cluster_index,				//!< cluster index
					size_t object_index						//!< object index
					) const = 0;
};

//! \class cluster_engine_factory
//! \brief class factory for clustering algorithm engine
class cluster_engine_factory
{
public:
	//! \brief caller is responsible for deleting cluster engine object
	static 
	cluster_engine* 
	get_cluster_engine();
};

#endif // _terimber_mstaccess_h_
