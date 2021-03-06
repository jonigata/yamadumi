/*!
  @file		partix_tetrahedral_mesh.hpp
  @brief	<�T�v>

  <����>
  $Id: partix_tetrahedral_mesh.hpp 252 2007-06-23 08:32:33Z naoyuki $
*/
#ifndef PARTIX_TETRAHEDRAL_MESH_HPP
#define PARTIX_TETRAHEDRAL_MESH_HPP

#include "partix_forward.hpp"
#include "partix_cloud.hpp"
#include "partix_geometry.hpp"
#include <map>

namespace partix {

template < class Traits >
class TetrahedralMesh {
public:
	typedef typename Traits::vector_traits	vector_traits;
	typedef typename Traits::real_type		real_type;
	typedef typename Traits::vector_type	vector_type;
	typedef typename Traits::index_type		index_type;
	typedef std::vector< index_type >		indices_type;
	typedef Volume< Traits >				volume_type;
	typedef Cloud< Traits >					cloud_type;
	typedef Point< Traits >					point_type;
	typedef std::vector< point_type >		points_type;
	typedef Face< Traits >					face_type;
	typedef Tetrahedron< Traits >			tetrahedron_type;

	struct edge_type {
		Edge< Traits >	indices;
		bool			border;
		real_type		t; // ("not collided"-"collided")�̌W��
		real_type		u;
		real_type		v;
		real_type		w;
		vector_type		collision_normal;
	};

	typedef std::vector< edge_type >		edges_type;
	typedef std::vector< face_type >		faces_type;
	typedef std::vector< tetrahedron_type > tetrahedra_type;

public:
	TetrahedralMesh()
	{
		volume_ = NULL;
		cloud_ = new Cloud< Traits >;
	}
	~TetrahedralMesh() { delete cloud_; }

	void add_point( const vector_type& v, real_type mass )
	{
		cloud_->add_point( v, mass );
	}
	void add_face( index_type i0, index_type i1, index_type i2 )
	{
		face_type f; f.i0 = i0; f.i1 = i1; f.i2 = i2;
		faces_.push_back( f );
	}
	void add_tetrahedron(
		index_type i0,
		index_type i1,
		index_type i2,
		index_type i3 )
	{
		tetrahedron_type t;
		t.i0 = i0;
		t.i1 = i1;
		t.i2 = i2;
		t.i3 = i3; 
		tetrahedra_.push_back( t );
	}

	void setup()
	{
		{
			edges_.clear();

			std::map< index_type, std::set< index_type > > s;
			for( typename tetrahedra_type::const_iterator i =
					 tetrahedra_.begin() ;
				 i != tetrahedra_.end() ;
				 ++i ) { 
				const tetrahedron_type& t = *i;
				make_edge( s, t.i0, t.i1 );
				make_edge( s, t.i0, t.i2 );
				make_edge( s, t.i0, t.i3 );
				make_edge( s, t.i1, t.i2 );
				make_edge( s, t.i1, t.i3 );
				make_edge( s, t.i2, t.i3 );
			}
		}

		{
			std::set< index_type > s;
			for( typename tetrahedra_type::const_iterator i =
					 tetrahedra_.begin() ;
				 i != tetrahedra_.end() ;
				 ++i ) { 
				const tetrahedron_type& t = *i;
				s.insert( t.i0 );
				s.insert( t.i1 );
				s.insert( t.i2 );
				s.insert( t.i3 );
			}

			indices_.clear();
			for( typename std::set< index_type>::const_iterator i =
					 s.begin() ;
				 i != s.end();
				 ++i ) {
				indices_.push_back( *i );
			}
		}						 

		{
			points_type& points = cloud_->get_points();

			average_edge_length_ = 0;
			for( typename edges_type::const_iterator i =
					 edges_.begin() ;
				 i != edges_.end() ;
				 ++i ) {
				const edge_type& e = *i;
				index_type ei0 = e.indices.i0;
				index_type ei1 = e.indices.i1;

#ifdef _WINDOWS
#if 1
				if( ei0 < 0 || int( points.size() ) <= ei0 ) {
					DebugBreak();
				}
				if( ei1 < 0 || int( points.size() ) <= ei1 ) {
					DebugBreak();
				}
#else
				// WORKAROUND bug
				if( ei0 < 0 || int( points.size() ) <= ei0 ) {
					continue;
				}
				if( ei1 < 0 || int( points.size() ) <= ei1 ) {
					continue;
				}
#endif
#endif
				const point_type& v0 = points[ei0];
				const point_type& v1 = points[ei1];

				average_edge_length_ += vector_traits::length(
					v0.source_position - v1.source_position
					);
			}
			average_edge_length_ /= real_type( edges_.size() );
		}
	}

	real_type get_average_edge_length()
	{
		return average_edge_length_;
	}

	cloud_type*				get_cloud() { return cloud_; }
	points_type&			get_points() { return cloud_->get_points(); }
	edges_type&				get_edges() { return edges_; }
	faces_type&				get_faces() { return faces_; }
	tetrahedra_type&		get_tetrahedra() { return tetrahedra_; }
	indices_type&			get_indices() { return indices_; }

	void			set_volume( volume_type* p ) { volume_ = p; }
	volume_type*	get_volume() { return volume_; }

private:
	TetrahedralMesh( const TetrahedralMesh& ){}
	void operator=( const TetrahedralMesh& ){}

private:
	void make_edge(
		std::map< index_type, std::set< index_type > >& s,
		index_type i0,
		index_type i1 )
	{
		if( i1 < i0 ) { std::swap( i0, i1 ); }
				
		std::set< index_type >& ss = s[i0];
		if( ss.find( i1 ) == ss.end() ) {
			edge_type e;
			e.indices.i0 = i0;
			e.indices.i1 = i1;
			e.border = false;
			edges_.push_back( e );
			ss.insert( i1 );
		}
	}

private:
	volume_type*	volume_;
	cloud_type*		cloud_;
	edges_type		edges_;
	faces_type		faces_;
	indices_type	indices_;
	tetrahedra_type tetrahedra_;
	real_type		average_edge_length_;
		

	//template < class T > friend class SoftVolume;
	//template < class T > friend class VertexTetrahedronSpatialHash;
};

};

#endif // PARTIX_TETRAHEDRAL_MESH_HPP
