/*!
  @file     partix_math.hpp
  @brief    <概要>

  <説明>
  $Id: partix_math.hpp 252 2007-06-23 08:32:33Z naoyuki $
*/
#ifndef PARTIX_MATH_HPP
#define PARTIX_MATH_HPP

namespace partix {

template < class Real >
class quaternion {
public:
	Real    u;
	Real    x;
	Real    y;
	Real    z;

	quaternion(){}
	quaternion( Real uu, Real xx, Real yy, Real zz )
		: u( uu ), x( xx ), y( yy ), z( zz ) {}

	quaternion< Real > operator*( const quaternion< Real >& q ) const
	{
		return quaternion< Real >(
			u * q.u - x * q.x - y * q.y - z * q.y,
			u * q.x + x * q.u + y * q.z - z * q.y,
			u * q.y + y * q.u + z * q.x - x * q.z,
			u * q.z + z * q.u + x * q.y - y * q.x );
	}

	quaternion< Real >& operator*=( const quaternion< Real >& q )
	{
		*this = *this * q;
		return *this;
	}

	quaternion< Real > normalize()
	{
		Real m = Real( 1.0 ) / module();
		return quaternion< Real > ( u * m, x * m, y * m, z * m );
	}

	Real module() { return Real( sqrt( u * u + x * x + y * y + z * z ) ); }

	void make_matrix( Real m[9] )
	{
		Real xx = x * x;
		Real xy = x * y;
		Real xz = x * z;
		Real xu = x * u;
		Real yy = y * y;
		Real yz = y * z;
		Real yu = y * u;
		Real zz = z * z;
		Real zu = z * u;

		Real r1 = Real( 1.0 );
		Real r2 = Real( 2.0 );

		m[0] = r1 - r2 * ( yy + zz );
		m[1] = r2 * ( xy + zu );
		m[2] = r2 * ( xz - yu );
		m[3] = r2 * ( xy - zu );
		m[4] = r1 - r2 * ( xx + zz );
		m[5] = r2 * ( yz + xu );
		m[6] = r2 * ( xz + yu );
		m[7] = r2 * ( yz - xu );
		m[8] = r1 - r2 * ( xx + yy );
	}
};

template < class Traits >
struct math {
	typedef typename Traits::vector_traits  vector_traits;
	typedef typename Traits::vector_type    vector_type;
	typedef typename Traits::real_type      real_type;

	// syntax sugar
	struct matrix {
	public:
		matrix( real_type* p ) : p_( p ) {}
		real_type* operator[]( int i ) { return p_ + i * 3; }
	private:
		real_type* p_;
	};
	struct const_matrix {
	public:
		const_matrix( const real_type* p ) : p_( p ) {}
		const real_type* operator[]( int i ) { return p_ + i * 3; }
	private:
		const real_type* p_;
	};
        
	static void barycentric(
		const vector_type& a,
		const vector_type& b,
		const vector_type& c,
		const vector_type& p,
		real_type& u,
		real_type& v,
		real_type& w )
	{
		vector_type v0 = b - a;
		vector_type v1 = c - a;
		vector_type v2 = p - a;
		real_type d00 = dot( v0, v0 );
		real_type d01 = dot( v0, v1 );
		real_type d11 = dot( v1, v1 );
		real_type d20 = dot( v2, v0 );
		real_type d21 = dot( v2, v1 );
		real_type denom = d00 * d11 - d01 * d01;
		if( denom < real_type( 0.0000001 ) ) {
			u = 0;
			v = 0;
			w = 0;
		} else {
			real_type idenom = real_type( 1.0 ) / denom;
			v = ( d11 * d20 - d01 * d21 ) * idenom;
			w = ( d00 * d21 - d01 * d20 ) * idenom;
			u = 1.0f - v - w;
		}
	}

	static real_type clip( real_type x )
	{
		if( x < 0 ) { return 0; }
		if( real_type( 1.0 ) < x ) { return real_type( 1.0 ); }
		return x;
	}

	static real_type dot( const vector_type& u, const vector_type& v )
	{
		return u.x * v.x + u.y * v.y + u.z * v.z;
	}

	static vector_type cross( const vector_type& u, const vector_type& v )
	{
		return vector_traits::make_vector(
			u.y * v.z - u.z * v.y,
			-u.x * v.z + u.z * v.x,
			u.x * v.y - u.y * v.x );
	}

	static real_type length( const vector_type& u )
	{
		return real_type( sqrt( length_sq( u ) ) );
	}

	static real_type length_sq( const vector_type& u )
	{
		return real_type( u.x * u.x + u.y * u.y + u.z * u.z );
	}

	static vector_type normalize( const vector_type& u )
	{
		return u * ( real_type(1) / length( u ) );
	}

	static void normalize_f( vector_type& u )
	{
		u *= real_type(1) / length( u );
	}

	static void make_identity( real_type* src )
	{
		const real_type c1 = real_type( 1 );
		matrix m( src );
		m[0][0] = c1; m[0][1] =  0; m[0][2] =  0;
		m[1][0] =  0; m[1][1] = c1; m[1][2] =  0;
		m[2][0] =  0; m[2][1] =  0; m[2][2] = c1;
	}

	static void copy_matrix( real_type* dst, const real_type* src )
	{
		for( int i = 0 ; i < 9 ; i++ ) { dst[i] = src[i]; }
	}

	static void print_matrix( const real_type* m )
	{
#ifdef _WINDOWS
		for( int i = 0 ; i < 9 ; i++ ) {
			char buffer[256];
			sprintf( buffer, "%f%c", m[i],
					 (i % 3 != 2 ? ',' : '\n') );
			OutputDebugStringA(buffer);
		}
		OutputDebugStringA( "\n" );
#endif
	}

	static void inverse_matrix( real_type* dst, const real_type* src )
	{
		real_type d = determinant_matrix( src );
		if( d == 0 ) { d = 1; }
		real_type di = 1.0f / d;

		matrix dm( dst );
		const_matrix sm( src );

		// a11*a22 - a12*a21  a02*a21 - a01*a22  a01*a12 - a02*a11
		// a12*a20 - a10*a22  a00*a22 - a02*a20  a02*a10 - a00*a12
		// a10*a21 - a11*a20  a01*a20 - a00*a21  a00*a11 - a01*a10

		dm[0][0] = ( sm[1][1] * sm[2][2] - sm[1][2] * sm[2][1] ) * di;
		dm[0][1] = ( sm[0][2] * sm[2][1] - sm[0][1] * sm[2][2] ) * di;
		dm[0][2] = ( sm[0][1] * sm[1][2] - sm[0][2] * sm[1][1] ) * di;
		dm[1][0] = ( sm[1][2] * sm[2][0] - sm[1][0] * sm[2][2] ) * di;
		dm[1][1] = ( sm[0][0] * sm[2][2] - sm[0][2] * sm[2][0] ) * di;
		dm[1][2] = ( sm[0][2] * sm[1][0] - sm[0][0] * sm[1][2] ) * di;
		dm[2][0] = ( sm[1][0] * sm[2][1] - sm[1][1] * sm[2][0] ) * di;
		dm[2][1] = ( sm[0][1] * sm[2][0] - sm[0][0] * sm[2][1] ) * di;
		dm[2][2] = ( sm[0][0] * sm[1][1] - sm[0][1] * sm[1][0] ) * di;
	}

	static void multiply_matrix(
		real_type* dst,
		const real_type* x,
		const real_type* y )
	{
		dst[0] = x[0] * y[0] + x[1] * y[3] + x[2] * y[6];
		dst[1] = x[0] * y[1] + x[1] * y[4] + x[2] * y[7];
		dst[2] = x[0] * y[2] + x[1] * y[5] + x[2] * y[8];

		dst[3] = x[3] * y[0] + x[4] * y[3] + x[5] * y[6];
		dst[4] = x[3] * y[1] + x[4] * y[4] + x[5] * y[7];
		dst[5] = x[3] * y[2] + x[4] * y[5] + x[5] * y[8];

		dst[6] = x[6] * y[0] + x[7] * y[3] + x[8] * y[6];
		dst[7] = x[6] * y[1] + x[7] * y[4] + x[8] * y[7];
		dst[8] = x[6] * y[2] + x[7] * y[5] + x[8] * y[8];
	}

	static void transpose_matrix( real_type* dst, const real_type* src )
	{
		dst[0] = src[0]; dst[1] = src[3]; dst[2] = src[6];
		dst[3] = src[1]; dst[4] = src[4]; dst[5] = src[7]; 
		dst[6] = src[2]; dst[7] = src[5]; dst[8] = src[8];
	}

	static void add_matrix(
		real_type* dst,
		const real_type* src1,
		const real_type* src2 )
	{
		for( int i = 0 ; i < 9 ; i++ ) { dst[i] = src1[i] + src2[i]; }
	}

	static void multiply_matrix(
		real_type* dst,
		const real_type* src,
		real_type s )
	{
		for( int i = 0 ; i < 9 ; i++ ) { dst[i] = src[i] * s; }
	}

	static real_type determinant_matrix( const real_type* src )
	{
		real_type d =
			src[0] * src[4] * src[8] -
			src[0] * src[7] * src[5] +
			src[3] * src[7] * src[2] -
			src[3] * src[1] * src[8] +
			src[6] * src[1] * src[5] -
			src[6] * src[4] * src[2];
		return d;
	}
        
	static real_type aa( const real_type* s, int r, int c )
	{
		return s[r*3+c];
	}

	static void turn_inside_out_matrix( real_type* m )
	{
		real_type t;
		t = m[0]; m[0] = m[3]; m[3] = t;
		t = m[1]; m[1] = m[4]; m[4] = t;
		t = m[2]; m[2] = m[5]; m[5] = t;
	}

#if 1
// Denman-Beavers method
	static void sqrt_matrix( real_type dst[9], const real_type src[9] )
	{
		real_type* Y = dst; memcpy( Y, src, sizeof( real_type ) * 9 );
		real_type Z[9] = { 1.0f, 0, 0, 0, 1.0f, 0, 0, 0, 1.0f };

		for( int i = 0 ; i < 10 ; i++ ) {
			real_type Yi[9]; inverse_matrix( Yi, Y );
			real_type Zi[9]; inverse_matrix( Zi, Z );
			for( int j = 0 ; j < 9 ; j++ ) {
				Y[j] = ( Y[j] + Zi[j] ) * 0.5f;
				Z[j] = ( Z[j] + Yi[j] ) * 0.5f;
			}
		}
	}
#else
	static void jacobi_rotation( real_type a[9], real_type x[9] )
	{
		const real_type epsilon = real_type(1.0E-7);
		const real_type PI = real_type(3.141592654);

		//   初期値設定
		for( int i = 0 ; i < 3 ; i++ ){
			for( int j = 0 ; j< 3 ; j++ ) { 
				if( i == j ) x[i*3+j] = 1.0 ;
				else  x[i*3+j] = 0.0 ; 
			}
		}
        
		//  反復計算
		int iter = 0;
		int ip = 0, jp = 0;
		for(;;) {
			//   非対角要素の最大値探索
			real_type amax = 0;
			for( int k = 0 ; k < 3-1 ; k++ ){
				for( int m = k + 1 ; m < 3 ; m++ ){
					real_type w = std::abs( a[k*3+m] );
					if( amax < w ) {
						ip = k;
						jp = m;
						amax = w;
					}
				}
			}

			//  収束判定
			if( amax <= epsilon ){
				return;
			} else if( 200 <= iter ){
				return;
			} else {
				real_type aii = a[ip*3+ip];
				real_type aij = a[ip*3+jp];
				real_type ajj = a[jp*3+jp];
                        
				//   回転角度計算
				real_type theta;
				if( std::abs( aii-ajj ) < epsilon ){
					theta = 0.25f * PI * aij /
						std::abs( aij );
				} else {
					theta = 0.5f * atanf(
						2.0f * aij / ( aii - ajj ) );
				}
				real_type co = cosf( theta );
				real_type si = sinf( theta );

				//  相似行列の計算
				for( int k = 0 ; k < 3 ; k++ ) {
					if( k != ip && k != jp ) {
						real_type aik = a[ip*3+k];
						real_type ajk = a[jp*3+k];
						real_type w;
						w = aik * co + ajk * si;
						a[ip*3+k] = w;
						a[k*3+ip] = w;
						w = -aik * si + ajk * co;
						a[jp*3+k] = w;
						a[k*3+jp] = w;
					}
				}
				a[ip*3+ip] =
					aii * co * co +
					( 2.0f * aij * co + ajj * si ) * si;
				a[jp*3+jp] =
					aii * si * si -
					( 2.0f * aij * si - ajj * co ) * co;
				a[ip*3+jp] = 0.0;
				a[jp*3+ip] = 0.0;

				// 固有ベクトルの計算
				for( int k = 0 ; k < 3 ; k++ ){
					real_type w = x[k*3+ip];
					x[k*3+ip] =  w * co + x[k*3+jp] * si;
					x[k*3+jp] = -w * si + x[k*3+jp] * co;
				}
			}
			iter++;
		}
	}

	static void sqrt_matrix( real_type* dst, const real_type* src )
	{
		real_type D[9];
		real_type E[9];
		real_type ET[9];

		memcpy( D, src, sizeof( D ) );
		jacobi_rotation( D, E );
		transpose_matrix( ET, E );

		for( int i = 0 ; i < 3 ; i++ ) {
			D[i*3+i] = sqrtf( D[i*3+i] );
		}

		real_type T[9];
		multiply_matrix( T, D, ET );
		multiply_matrix( dst, E, T );
	}
#endif

	static void transform_vector(
		vector_type& dst,
		const real_type* m,
		const vector_type& src )
	{
		vector_traits::x( dst, m[0] * src.x + m[1] * src.y + m[2] * src.z );
		vector_traits::y( dst, m[3] * src.x + m[4] * src.y + m[5] * src.z );
		vector_traits::z( dst, m[6] * src.x + m[7] * src.y + m[8] * src.z );
	}

	static real_type epsilon() { return real_type( 0.000001 ); }
	static real_type real_min()
	{
		return -( std::numeric_limits< real_type >::max )();
	}
	static real_type real_max()
	{
		return ( std::numeric_limits< real_type >::max )();
	}

	static vector_type vector_min()
	{
		real_type x = (std::numeric_limits< real_type >::max)();
		return vector_traits::make_vector( -x, -x, -x );
	}
	static vector_type vector_max()
	{
		real_type x = (std::numeric_limits< real_type >::max)();
		return vector_traits::make_vector( x, x, x );
	}
	static vector_type vector_zero()
	{
		return vector_traits::make_vector( 0, 0, 0 );
	}

	template < class T > 
	static void mount( T& v, T& e, const T& d )
	{
		// エラー訂正和
		T t1 = d + e;
		T t2 = v + t1;
		e = t1 - ( t2 - v );
		v = t2;
	}

	static void get_tetrahedron_bb( const vector_type& v0,
									const vector_type& v1,
									const vector_type& v2,
									const vector_type& v3,
									vector_type& bbmin,
									vector_type& bbmax )
	{
		minmax4( v0.x, v1.x, v2.x, v3.x, bbmin.x, bbmax.x );
		minmax4( v0.y, v1.y, v2.y, v3.y, bbmin.y, bbmax.y );
		minmax4( v0.z, v1.z, v2.z, v3.z, bbmin.z, bbmax.z );
	}

	static void get_triangle_bb(  const vector_type& v0,
								  const vector_type& v1,
								  const vector_type& v2,
								  vector_type& bbmin,
								  vector_type& bbmax  )
	{
		minmax3( v0.x, v1.x, v2.x, bbmin.x, bbmax.x );
		minmax3( v0.y, v1.y, v2.y, bbmin.y, bbmax.y );
		minmax3( v0.z, v1.z, v2.z, bbmin.z, bbmax.z );
	}

	static void get_segment_bb(  const vector_type& v0,
								 const vector_type& v1,
								 vector_type& bbmin,
								 vector_type& bbmax  )
	{
		minmax2( v0.x, v1.x, bbmin.x, bbmax.x );
		minmax2( v0.y, v1.y, bbmin.y, bbmax.y );
		minmax2( v0.z, v1.z, bbmin.z, bbmax.z );
	}

	static void update_bb( vector_type& bbmin,
						   vector_type& bbmax,
						   const vector_type& p )
	{
		update_minmax( bbmin.x, bbmax.x, p.x );
		update_minmax( bbmin.y, bbmax.y, p.y );
		update_minmax( bbmin.z, bbmax.z, p.z );
	}

	static void update_minmax( real_type& mn, real_type& mx, real_type v )
	{
		if( v < mn ) { mn = v; }
		if( mx < v ) { mx = v; }
	}

	static void minmax4(
		real_type a, real_type b, real_type c, real_type d,
		real_type& minv, real_type& maxv )
	{
		real_type minab, maxab, mincd, maxcd;
		minab = b; maxab = a; if( a < b ) { minab = a; maxab = b; }
		mincd = d; maxcd = c; if( c < d ) { mincd = c; maxcd = d; }
		minv = mincd; if( minab < mincd ) { minv = minab; }
		maxv = maxab; if( maxab < maxcd ) { maxv = maxcd; }
	}

	static void minmax3(
		real_type a, real_type b, real_type c,
		real_type& minv, real_type& maxv )
	{
		real_type minab, maxab;
		minab = b; maxab = a;   if( a < b ) { minab = a; maxab = b; }
		minv = c;               if( minab < c ) { minv = minab; }
		maxv = maxab;           if( maxab < c ) { maxv = c; }
	}

	static void minmax2(
		real_type a, real_type b,
		real_type& minv, real_type& maxv )
	{
		if( a < b ) { minv = a; maxv = b; return; }
		minv = b; maxv = a;
	}

	static void minmax4_vector(
		const vector_type& a,
		const vector_type& b,
		const vector_type& c,
		const vector_type& d,
		vector_type& minv,
		vector_type& maxv )
	{
		minmax4( a.x, b.x, c.x, d.x, minv.x, maxv.x );
		minmax4( a.y, b.y, c.y, d.y, minv.y, maxv.y );
		minmax4( a.z, b.z, c.z, d.z, minv.z, maxv.z );
	}

	static void minmax3_vector(
		const vector_type& a,
		const vector_type& b,
		const vector_type& c,
		vector_type& minv,
		vector_type& maxv )
	{
		minmax3( a.x, b.x, c.x, minv.x, maxv.x );
		minmax3( a.y, b.y, c.y, minv.y, maxv.y );
		minmax3( a.z, b.z, c.z, minv.z, maxv.z );
	}

	static void minmax2_vector(
		const vector_type& a,
		const vector_type& b,
		vector_type& minv,
		vector_type& maxv )
	{
		minmax2( a.x, b.x, minv.x, maxv.x );
		minmax2( a.y, b.y, minv.y, maxv.y );
		minmax2( a.z, b.z, minv.z, maxv.z );
	}

	static void point_to_triangle_distance( 
        vector_type& q,
        vector_type& uvw, 
        const vector_type& a,
        const vector_type& b,
        const vector_type& c,
        const vector_type& p )
	{
        vector_type ab = b - a;
        vector_type ac = c - a;
        vector_type ap = p - a;

        // PがAの外側の頂点領域にあるかどうかチェック
        float d1 = dot( ab, ap );
        float d2 = dot( ac, ap );
        if( d1 <= 0.0f && d2 <= 0.0f ) {
			q = a; uvw = vector_type( 1, 0, 0 ); return;
		} // 重心座標(1,0,0)
                
        // PがBの外側の頂点領域の中にあるかどうかチェック
        vector_type bp = p - b;
        float d3 = dot( ab, bp );
        float d4 = dot( ac, bp );
        if( d3 >= 0.0f && d4 <= d3 ) {
			q = b; uvw = vector_type( 0, 1, 0 ); return;
		} // 重心座標(0,1,0)

        // PがABの辺領域の中にあるかどうかチェックし、
		// あればPのAB上に対する射影を返す
        float vc = d1 * d4 - d3 * d2;
        if( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f ) {
			float v = d1 / ( d1 - d3 );
			q = a + v * ab; // 重心座標(1-v,v,0)
			uvw = vector_type( 1-v, v, 0 ); 
			return;
        }

        // PがCの外側の頂点領域の中にあるかどうかチェック
        vector_type cp = p - c;
        float d5 = dot( ab, cp );
        float d6 = dot( ac, cp );
        if( d6 >= 0.0f && d5 <= d6 ) {
			q = c; uvw = vector_type( 0, 0, 1 ); return;
		} // 重心座標(0,0,1)

        // PがACの辺領域の中にあるかどうかチェックし、
		// あればPのAB上に対する射影を返す
        float vb = d5 * d2 - d1 * d6;
        if( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f ) {
			float w = d2 / ( d2 - d6 );
			q = a + w * ac;
			uvw = vector_type( 1-w, 0, w );  // 重心座標(1-w,0,w)
			return;
        }

        // PがBCの外側の頂点領域の中にあるかどうかチェック
        float va = d3 * d6 - d5 * d4;
        if( va <= 0.0f && ( d4 - d3 ) >= 0.0f && ( d5 - d6 ) >= 0.0f ) {
			float w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
			q = b + w * ( c - b );
			uvw = vector_type( 0, 1-w, w );  // 重心座標(0,1-w,w)
			return;
        }

        // Pは面領域の中にある Qをその重心座標(u,v,w)を用いて計算
        float denom = 1.0f / ( va + vb + vc );
        float v = vb * denom;
        float w = vc * denom;
        q = a + ab * v + ac * w;
		// = u * a + v * b + w * c, u = va * denom = 1.0f - v - w
        uvw = vector_type( 1.0f - v - w, v, w );
        return;
	}

	static bool test_plane_segment( const vector_type& plane_position,
									const vector_type& plane_normal,
									const vector_type& s0,
									const vector_type& s1,
									real_type& t)
	{
		vector_type ab = s1 - s0;
		t = ( dot( plane_position, plane_normal ) -
			  dot( plane_normal, s0 ) ) /
			dot( plane_normal, ab );
                
		return real_type( 0 ) <= t && t <= real_type( 1 );
	}

	static bool test_aabb_segment( const vector_type& bbmin,
								   const vector_type& bbmax,
								   const vector_type& s0,
								   const vector_type& s1 )
	{
		vector_type c = ( bbmin + bbmax ) * real_type( 0.5 );
		vector_type e = bbmax - c;

		vector_type m = ( s0 + s1 ) * real_type( 0.5 );
		vector_type d = s1 - m;
		m = m - c;

		real_type adx = std::abs( d.x );
		if( std::abs( m.x ) > e.x + adx ) return false;
		real_type ady = std::abs( d.y );
		if( std::abs( m.y ) > e.y + ady ) return false;
		real_type adz = std::abs( d.z );
		if( std::abs( m.z ) > e.z + adz ) return false;

		adx += epsilon(); ady += epsilon(); adz += epsilon();

		if( std::abs( m.y * d.z - m.z * d.y ) > e.y * adz + e.z * ady )
			return false;
		if( std::abs( m.z * d.x - m.x * d.z ) > e.x * adz + e.z * adx )
			return false;
		if( std::abs( m.x * d.y - m.y * d.x ) > e.x * ady + e.y * adx )
			return false;

		return true;
	}

	static bool test_aabb_plane( const vector_type& bbmin,
								 const vector_type& bbmax,
								 const vector_type& plane_position,
								 const vector_type& plane_normal )
	{
		vector_type c = ( bbmin + bbmax ) * real_type( 0.5 );
		vector_type e = bbmax - c;

		real_type r =
			e.x * std::abs( plane_normal.x ) +
			e.y * std::abs( plane_normal.y ) +
			e.z * std::abs( plane_normal.z );

		real_type s =
			dot( plane_normal, c ) -
			dot( plane_normal, plane_position );

		return std::abs( s ) <= r;
	}
                                        
	static bool test_aabb_aabb(
		const vector_type& mn0, const vector_type& mx0,
		const vector_type& mn1, const vector_type& mx1 )
	{
		if( mx0.x < mn1.x || mx1.x < mn0.x ) { return false; }
		if( mx0.y < mn1.y || mx1.y < mn0.y ) { return false; }
		if( mx0.z < mn1.z || mx1.z < mn0.z ) { return false; }
		return true;
	}

	static bool test_aabb_point(
		const vector_type& mn0, const vector_type& mx0,
		const vector_type& q )
	{
		return  mn0.x <= q.x && q.x <= mx0.x &&
			mn0.y <= q.y && q.y <= mx0.y &&
			mn0.z <= q.z && q.z <= mx0.z;
	}

	static bool test_segment_triangle( const vector_type& r0,
									   const vector_type& r1,
									   const vector_type& v0,
									   const vector_type& v1,
									   const vector_type& v2,
									   vector_type& uvt )
	{
		vector_type dir = r1 - r0;

		/* find vectors for two edges sharing vert0 */
		vector_type e1 = v1 - v0;
		vector_type e2 = v2 - v0;

		/* begin calculating determinant - also used to calculate U parameter */
		vector_type pvec = math< Traits >::cross( dir, e2 );

		/* if determinant is near zero, ray lies in plane of triangle */
		real_type det = math< Traits >::dot( e1, pvec );

		if ( det < math< Traits >::epsilon() )
			return false;

		/* calculate distance from vert0 to ray origin */
		vector_type tvec = r0 - v0;

		/* calculate U parameter and test bounds */
		uvt.x = math< Traits >::dot( tvec, pvec );
		if ( uvt.x < 0 || uvt.x > det )
			return false;

		/* prepare to test V parameter */
		vector_type qvec = math< Traits >::cross( tvec, e1 );

		/* calculate V parameter and test bounds */
		uvt.y = math< Traits >::dot( dir, qvec );
		if ( uvt.y < 0 || uvt.x + uvt.y > det)
			return false;

		/* calculate t, scale parameters, ray intersects triangle */
		real_type z = math< Traits >::dot( e2, qvec );
		if( z < 0 || det < z ) 
			return false;

		uvt.z = z;
		uvt *= real_type( 1.0 ) / det;

		return true;
	}
                                        
	static bool test_ray_triangle( const vector_type& r0,
								   const vector_type& r1,
								   const vector_type& v0,
								   const vector_type& v1,
								   const vector_type& v2,
								   vector_type& uvt )
	{

		vector_type dir = r1 - r0;

		/* find vectors for two edges sharing vert0 */
		vector_type e1 = v1 - v0;
		vector_type e2 = v2 - v0;

		/* begin calculating determinant - also used to calculate U parameter */
		vector_type pvec = math< Traits >::cross( dir, e2 );

		/* if determinant is near zero, ray lies in plane of triangle */
		real_type det = math< Traits >::dot( e1, pvec );

		if ( det < math< Traits >::epsilon() )
			return false;

		/* calculate distance from vert0 to ray origin */
		vector_type tvec = r0 - v0;

		/* calculate U parameter and test bounds */
		uvt.x = math< Traits >::dot( tvec, pvec );
		if ( uvt.x < 0 || uvt.x > det )
			return false;

		/* prepare to test V parameter */
		vector_type qvec = math< Traits >::cross( tvec, e1 );

		/* calculate V parameter and test bounds */
		uvt.y = math< Traits >::dot( dir, qvec );
		if ( uvt.y < 0 || uvt.x + uvt.y > det)
			return false;

		/* calculate t, scale parameters, ray intersects triangle */
		real_type z = math< Traits >::dot( e2, qvec );
                
		if( z < 0 ) // 半直線
			return false;

		uvt.z = z;
		uvt *= real_type( 1.0 ) / det;

		return true;
	}

	static bool test_sphere_triangle(
		const vector_type&	center,
		real_type			radius,
		const vector_type&	v0,
		const vector_type&	v1,
		const vector_type&	v2,
		vector_type& uvt )
	{
		vector_type q;
		point_to_triangle_distance(
			q,
			uvt,
			v0,
			v1,
			v2,
			center);
		return length_sq( q - center ) <= square( radius );
	}
};

}

#endif // PARTIX_MATH_HPP

