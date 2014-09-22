/*!
  @file		voxel_traverser.hpp
  @brief	<�T�v>

  <����>
  $Id: voxel_traverser.hpp 251 2007-06-22 15:15:48Z naoyuki $
*/
#ifndef VOXEL_TRAVERSER_HPP
#define VOXEL_TRAVERSER_HPP

#include <limits>
#include <math.h>
#include <iostream>

template < class Real, class Vector >
class voxel_traverser {
public:
	typedef Real	real_type;
	typedef Vector	vector_type;

public:
	voxel_traverser(
		const vector_type& v0,
		const vector_type& v1,
		real_type gridsize )
		: v0_( v0 ),
		  v1_( v1 ),
		  gridsize_( gridsize ),
		  rgridsize_( real_type( 1.0 ) / gridsize )
	{
		vector_type v = v1 - v0;

		x_ = coord( v0_.x );
		y_ = coord( v0_.y );
		z_ = coord( v0_.z );

		endx_ = coord( v1_.x );
		endy_ = coord( v1_.y );
		endz_ = coord( v1_.z );

		stepx_ = sgn( v.x );
		stepy_ = sgn( v.y );
		stepz_ = sgn( v.z );

		set_tmax( tmaxx_, v.x, v0.x, stepx_ );
		set_tmax( tmaxy_, v.y, v0.y, stepy_ );
		set_tmax( tmaxz_, v.z, v0.z, stepz_ );

		tdeltax_ = std::abs( gridsize_ / v.x );
		tdeltay_ = std::abs( gridsize_ / v.y );
		tdeltaz_ = std::abs( gridsize_ / v.z );
		if( x_ == endx_ ) {
			tdeltax_ = 0;
			tmaxx_ = (std::numeric_limits< real_type >::max)();
		}
		if( y_ == endy_ ) {
			tdeltay_ = 0;
			tmaxy_ = (std::numeric_limits< real_type >::max)();
		}
		if( z_ == endz_ ) {
			tdeltaz_ = 0;
			tmaxz_ = (std::numeric_limits< real_type >::max)();
		}

		ok_ = true;
	}
	~voxel_traverser(){}

	bool operator()( int& x, int& y, int& z )
	{
		if( !ok_ ) { return false; }

		x = x_;
		y = y_;
		z = z_;
		if( reach( x_, stepx_, endx_ ) &&
			reach( y_, stepy_, endy_ ) &&
			reach( z_, stepz_, endz_ ) ) {
			ok_ = false;
		}
				
		if( tmaxx_ < tmaxy_ ) {
			if( tmaxx_	< tmaxz_ ) {
				proceed( x_, stepx_, tmaxx_, tdeltax_ );
			} else {
				proceed( z_, stepz_, tmaxz_, tdeltaz_ );
			}
		} else {
			if( tmaxy_ < tmaxz_ ) {
				proceed( y_, stepy_, tmaxy_, tdeltay_ );
			} else {
				proceed( z_, stepz_, tmaxz_, tdeltaz_ );
			}
		}
		return true;
	}

private:
	int coord( real_type a ) { return int( floor( a * rgridsize_ ) ); }

	int sgn( real_type x )
	{
		if( x < 0 ) { return -1; }
		else if( 0 < x ) { return 1; }
		else { return 0; }
	}

/*
  bool reach( int a, int step, int e )
  {
  return
  step == 0 ||
  ( step < 0 && a <= e ) ||
  ( 0 < step && e <= a );
  }
*/
	bool reach( int a, int step, int e ) { return e*step <= a*step; }

	real_type nearest_bound( int step, real_type a )
	{
		if( step < 0 ) {
			return floor( a * rgridsize_ ) * gridsize_;
		} else {
			return ceil( a * rgridsize_ ) * gridsize_;
		}
	}
	void set_tmax( real_type& tmax, real_type v, real_type v0, int step )
	{
		if( v == 0 ) {
			tmax = (std::numeric_limits< real_type >::max)();
		} else {
			tmax = std::abs( ( nearest_bound( step, v0 ) - v0 ) / v );
		}
	}

	void proceed( int& a, int step, real_type& tmax, real_type tdelta )
	{
		a += step;
		tmax += tdelta;
	}

	vector_type v0_, v1_;
	real_type gridsize_, rgridsize_;
	int x_, y_, z_, endx_, endy_, endz_, stepx_, stepy_, stepz_;
	real_type tmaxx_, tmaxy_, tmaxz_;
	real_type tdeltax_, tdeltay_, tdeltaz_;
	bool ok_;
		
};

#endif // VOXEL_TRAVERSER_HPP