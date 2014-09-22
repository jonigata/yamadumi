// 2014/09/22 Naoyuki Hirayama

#ifndef TRAITS_HPP_
#define TRAITS_HPP_

#include "geometry.hpp"
#include "mqoreader.hpp"
#include "partix/partix.hpp"
#include <fstream>

const float MIKU_MASS = 0.5f;
const float MIKU_SCALE = 0.02f;

/*===========================================================================*/
/*!
 * VectorTraits
 *
 *  vector traits
 */
/*==========================================================================*/

struct VectorTraits {
    typedef float   real_type;
    typedef Vector  vector_type;

    static real_type epsilon(){ return real_type( 0.000001f ); }
    static real_type x(const vector_type& v) { return v.x; }
    static real_type y(const vector_type& v) { return v.y; }
    static real_type z(const vector_type& v) { return v.z; }
    static void x(vector_type& v, real_type x) { v.x = x; }
    static void y(vector_type& v, real_type y) { v.y = y; }
    static void z(vector_type& v, real_type z) { v.z = z; }
    static vector_type make_vector(real_type x, real_type y, real_type z) {
        return Vector(x, y, z);
    }
    static real_type length_sq(const vector_type& v) {
        return ::length_sq(v);
    }
    static real_type length(const vector_type& v) {
        return ::length(v);
    }
};


/*===========================================================================*/
/*!
 * PartixTraits
 *
 *  partix用のtraitsクラス
 *  vector等はD3DXのものを使用
 */
/*==========================================================================*/

struct PartixTraits {
    typedef VectorTraits                vector_traits;
    typedef VectorTraits::real_type     real_type;
    typedef VectorTraits::vector_type   vector_type;
    typedef Matrix                      matrix_type;
    typedef int                         index_type;

    struct body_load_type {};
    struct block_load_type {};
    struct cloud_load_type {};
    struct point_load_type {};

    static float speed_drag_coefficient() { return  0.0001f; }
    //static float kinetic_friction() { return 40.0f; }
    static float kinetic_friction() { return 0.0f; }

    static float freeze_threshold_energy() { return 2.0f; }
    static float freeze_duration() { return 0.5f; }

    static float tick() { return 0.02f; }
    static void make_matrix(
        matrix_type& d,
        const real_type* s,
        const vector_type& t) {
        d.m00 = s[0]; d.m01 = s[3]; d.m02 = s[6]; d.m03 = 0;
        d.m10 = s[1]; d.m11 = s[4]; d.m12 = s[7]; d.m13 = 0;
        d.m20 = s[2]; d.m21 = s[5]; d.m22 = s[8]; d.m23 = 0;
        d.m30 = t.x;  d.m31 = t.y;  d.m32 = t.z;  d.m33 = 1;
    }
    static vector_type transform_vector(
        const matrix_type& m,
        const vector_type& v) {
        return v * m;
    }
};

typedef Vector                                  vector_type;
typedef partix::World<PartixTraits>             world_type;
typedef partix::Point<PartixTraits>             point_type;
typedef partix::Cloud<PartixTraits>             cloud_type;
typedef partix::Block<PartixTraits>             block_type;
typedef partix::Body<PartixTraits>              body_type;
typedef partix::SoftShell<PartixTraits>         softshell_type;
typedef partix::SoftVolume<PartixTraits>        softvolume_type;
typedef partix::BoundingPlane<PartixTraits>     plane_type;
typedef partix::TetrahedralMesh<PartixTraits>   tetra_type;
typedef partix::Face<PartixTraits>              face_type;

typedef std::shared_ptr<body_type>              body_ptr;
typedef std::shared_ptr<cloud_type>             cloud_ptr;
typedef std::shared_ptr<block_type>             block_ptr;
typedef std::shared_ptr<softvolume_type>        softvolume_ptr;

class PartixWorld {
public:
    PartixWorld() {
        build();
    }

    void restart() {
        world_->restart();
    }

    void update() {
        world_->update(PartixTraits::tick());
    }

    void build() {
        // ...world
        world_.reset(new world_type);

        // ...room
        for (int i = 0 ; i <6 ; i++) {
            typedef vector_type vec;

            const vec bounds[6][2] = {
                { vec(-1.0f, 0, 0), vec(5.0f, 0, 0), },
                { vec(1.0f, 0, 0), vec(-5.0f, 0, 0), },
                { vec(0, -1.0f, 0), vec(0, 5.0f, 0), },
                { vec(0, 1.0f, 0), vec(0, -5.0f, 0), },
                { vec(0, 0, -1.0f), vec(0, 0, 5.0f), },
                { vec(0, 0, 1.0f), vec(0, 0, -5.0f), },
            };

            body_ptr e(new plane_type(
                           bounds[i][1],
                           bounds[i][0]));
            bodies_.push_back(e);
            world_->add_body(e.get());
        }
    }

    body_ptr add_entity() {
        // ...entity
        vector_type o;
        o.x = float(rand() % 6001 - 3000) / 2000;
        o.y = float(rand() % 6001 - 3000) / 2000;
        o.z = float(rand() % 6001 - 3000) / 2000;

        // 作成
        softvolume_type* v = make_volume_body(MIKU_SCALE*100);

        // 硬さ、摩擦
        v->set_restore_factor( 0.3f );
        v->set_stretch_factor( 0.7f );
        for (auto& p: v->get_mesh()->get_points()) {
            p.friction = 0.8f;
        }
                                
        // 登録
        body_ptr e(v);
        e->teleport(o);
        e->set_auto_freezing(false);
        e->set_global_force(vector_type(0, -9.8, 0));
        bodies_.push_back(e);
        models_.push_back(e);
        world_->add_body(e.get());
        return e;
    }

private:
    softvolume_type* make_volume_body(float mag) {
        vector_type v0(0, 0, 0);

        tetra_type* e = new tetra_type;

        printf("A\n");
        // .node(頂点座標)読み込み
        {
            std::ifstream ifs("data/miku2_p.node");
            int node_count, dummy;
            ifs >> node_count >> dummy >> dummy >> dummy;
            for (int i = 0 ; i <node_count ; i++) {
                vector_type v;
                ifs >> dummy >> v.x >> v.y >> v.z;
                v.x *= mag;
                v.y *= mag;
                v.z *= mag;
                e->add_point(v, MIKU_MASS);
            }
        }

        printf("B\n");
        // .ele(tetrahedron)読み込み
        {
            std::ifstream ifs("data/miku2_p.ele");
            int node_count, dummy;
            ifs >> node_count >> dummy >> dummy;
            for (int i = 0 ; i <node_count ; i++) {
                int i0, i1, i2, i3;
                ifs >> dummy >> i0 >> i1 >> i2 >> i3;
                e->add_tetrahedron(i0, i1, i2, i3);
            }
        }

        printf("C\n");
        // .face(外接面)読み込み
        {
            std::ifstream ifs("data/miku2_p.face");
            int node_count, dummy;
            ifs >> node_count >> dummy;
            for (int i = 0 ; i <node_count ; i++) {
                int i0, i1, i2;
                ifs >> dummy >> i0 >> i1 >> i2 >> dummy;
                e->add_face(i0, i2, i1); // 反転
            }
        }

        printf("D\n");
        e->setup();
        softvolume_type* v = new softvolume_type;

        printf("E\n");
        v->set_mesh(e);

        printf("F\n");
        v->regularize();

        return v;
    }

private:
    std::unique_ptr<world_type> world_;
    std::vector<body_ptr>       bodies_;    // 全部
    std::vector<body_ptr>       models_;    // figure系だけ
    
};

#endif // TRAITS_HPP_
