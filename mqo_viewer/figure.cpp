// $Id: figure.cpp 32 2008-10-25 12:19:56Z Naoyuki.Hirayama $

#include "gl.hpp"
#include "figure.hpp"
#include "texture.hpp"

#define JSTEXTURE

/*===========================================================================*/
/*!
 * @class FigureImp
 * @brief 
 *
 * 
 */
/*==========================================================================*/

class FigureImp  {
private:
    struct Vertex {
        Vector      position;
        Vector      normal;
        Color       diffuse;
        float       u;
        float       v;
        Vertex() {}
        Vertex(const Vector& p, const Vector& n, const Color& c) {
            position = p;
            normal = n;
        }
    };

    typedef uint16_t Index;

    struct SubModel {
        std::vector<Vertex>     vertex_source;
        std::vector<Index>      index_source;
        GLuint                  vbo;
        GLuint                  ibo;
#ifdef JSTEXTURE
        std::string             texture;
#else
        GLuint                  texture;
#endif
        Color                   color;
    };                
    typedef std::vector<SubModel> submodels_type;

public:
    FigureImp() {}
    ~FigureImp() { clear(); }

    void clear() {
        clear_submodels();
    }

    bool empty() {
        return submodels_.empty();
    }

    void render() {
        for(SubModel& sm: submodels_) {
            render_submodel(sm);
        }
    }

    Matrix get_transform() {
        return Matrix::identity();
    }
    Color get_material_color() {
        return Color{ {1.0, 1.0, 0, 1.0} };
    }

    //
    // MQO
    //  このバージョンではテクスチャ、マテリアルなどを扱わない
    //
    void build_from_mqo(
        mqo_reader::document_type& doc, float scale, const Color& color) {

        // マテリアル
        for (const auto& ms: doc.materials) {
            SubModel m;
            m.vbo       = 0;
            m.ibo       = 0;
#ifdef JSTEXTURE
            m.texture   = "";
#else
            m.texture   = 0;
#endif
            m.color.rgba[0] = ms.color.red;
            m.color.rgba[1] = ms.color.green;
            m.color.rgba[2] = ms.color.blue;
            m.color.rgba[3] = ms.color.alpha;
            if (ms.texture != "") {
#ifdef JSTEXTURE
                m.texture = ms.texture;
                loadTexture(m.texture.c_str());
#else
                m.texture = build_texture(ms.texture.c_str());
#endif
            }

            submodels_.push_back(m);
        }
        {
            // default material
            SubModel m;
            m.vbo       = 0;
            m.ibo       = 0;
#ifdef JSTEXTURE
            m.texture   = "";
#else
            m.texture   = 0;
#endif
            m.color     = color;
            submodels_.push_back(m);
        }

        // 頂点, 面
        for (const auto& pair: doc.objects) {
            const mqo_reader::object_type& obj = pair.second;

            // dictionary:
            //  (source vertex index, uv ) => destination vertex index
            struct VertexKey {
                int     index;
                float   u;
                float   v;

                VertexKey(){}
                VertexKey(int aindex, float au, float av)
                    : index(aindex), u(au), v(av) {}

                bool operator<(const VertexKey& a) const {
                    if (index < a.index) { return true; }
                    if (a.index < index) { return false; }
                    if (u < a.u) { return true; }
                    if (a.u < u) { return false; }
                    return v < a.v;
                }
            };

            std::vector<std::map<VertexKey, int>> used_vertices;
            used_vertices.resize(submodels_.size());

            // マテリアルごとに使用頂点を分類
            for (const auto& face: obj.faces) {
                int material_index = face.material_index;
                if (material_index == -1) {
                    material_index = int(submodels_.size() - 1);
                }
                int i0 = face.vertex_indices[0];
                int i1 = face.vertex_indices[1];
                int i2 = face.vertex_indices[2];
                                
                std::map<VertexKey, int>& c = used_vertices[material_index];
                c[VertexKey(i0, face.uv[0].u, face.uv[0].v)] = -1; 
                c[VertexKey(i1, face.uv[1].u, face.uv[1].v)] = -1; 
                c[VertexKey(i2, face.uv[2].u, face.uv[2].v)] = -1; 
            }
                        
            // マテリアルごとに使われている頂点を追加
            size_t n = submodels_.size();
            for (size_t i = 0 ; i < n ; i++) {
                SubModel& m = submodels_[i];
                std::map<VertexKey, int>& c = used_vertices[i];

                int no = int(m.vertex_source.size());
                for (auto& j: c) {
                    const auto& src = obj.vertices[j.first.index];

                    Vertex dst;
                    dst.position = Vector(
                        src.x * scale,
                        src.y * scale,
                        src.z * -scale);
                    dst.normal = Vector(0, 0, 0);
                    dst.diffuse = m.color;
                    dst.u = j.first.u;
                    dst.v = j.first.v;
                    m.vertex_source.push_back(dst);

                    j.second = no++;
                }
            }

            // マテリアルごとに面を追加
            for (const auto& face: obj.faces) {
                int material_index = face.material_index;
                if (material_index == -1) {
                    material_index = int(submodels_.size()- 1);
                }
                                
                int i0 = face.vertex_indices[0];
                int i1 = face.vertex_indices[1];
                int i2 = face.vertex_indices[2];

                std::map<VertexKey, int>& c = used_vertices[material_index];
                int k0 = c[VertexKey(i0, face.uv[0].u, face.uv[0].v)];
                int k1 = c[VertexKey(i1, face.uv[1].u, face.uv[1].v)];
                int k2 = c[VertexKey(i2, face.uv[2].u, face.uv[2].v)];

                SubModel& m = submodels_[material_index];
                m.index_source.push_back(k0);
                m.index_source.push_back(k1);
                m.index_source.push_back(k2);

                Vertex& v0 = m.vertex_source[k0];
                Vertex& v1 = m.vertex_source[k1];
                Vertex& v2 = m.vertex_source[k2];
                Vector normal = cross(
                    v1.position - v0.position,
                    v2.position - v0.position);
                v0.normal += normal;
                v1.normal += normal;
                v2.normal += normal;
            }
        }

        // 法線後処理
        for (SubModel& m: submodels_) {
            for (auto& v: m.vertex_source) {
                normalize_f(v.normal);
            }
        }

        build_submodels();
    }        

private:
    void render_submodel(SubModel& m) {
        if (m.vbo ==0 || m.ibo == 0) {
            //printf("no contents\n");
            return ;
        }

        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);

        /* Set up the position of the attributes in the vertex buffer object */
        int stride = sizeof(Vertex);
        const float* p = 0;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, p + 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, p + 3);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, p + 6);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, p + 10);

        /* Enable the attributes */
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        //bindTexture(m.texture.c_str());
#ifdef JSTEXTURE
        bindTexture(m.texture.c_str());
#else
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m.texture);
#endif

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        /* Draw the triangle strips that comprise the gear */
        glDrawElements(
            GL_TRIANGLES, m.index_source.size(), GL_UNSIGNED_SHORT, nullptr);
        int error = glGetError();
        if (error != 0) {
            printf("error: %d\n", error);
        }

        /* Disable the attributes */
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void clear_submodels() {
        clear_vertex_buffers();
        submodels_.clear();
    }

    void clear_vertex_buffers() {
        for(SubModel& sm: submodels_) {
            // TODO: バッファの破棄
            if( sm.vbo ) { /* m.vb->Release() */; sm.vbo = 0; }
            if( sm.ibo ) { /* m.ib->Release() */; sm.ibo = 0; }
        }
    }

    void build_submodels() {
        clear_vertex_buffers();

        printf("submodel count = %d\n", submodels_.size());
        for(SubModel& sm: submodels_) {
            build_submodel(sm);
        }
    }

    void build_submodel( SubModel& m ) {
        const std::vector<Vertex>& vsrc = m.vertex_source;
        const std::vector<Index>&  isrc = m.index_source;

        if( vsrc.empty() || isrc.empty() ) {
            return;
        }

        printf("vertex_source.size() = %d\n", m.vertex_source.size());
        glGenBuffers(1, &m.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m.vertex_source.size() * sizeof(m.vertex_source[0]),
                     &m.vertex_source[0],
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &m.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m.index_source.size() * sizeof(m.index_source[0]),
                     &m.index_source[0],
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    GLuint build_texture(const char* filename) {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        const int TEX_W = 512;
	GLubyte pixels[TEX_W*TEX_W * 3];
        for (int i = 0 ; i < TEX_W*TEX_W ; i++) {
            pixels[i*3+0] = i & 255;
            pixels[i*3+1] = (i/2) & 255;
            pixels[i*3+2] = (i/3) & 255;
        }
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_W, TEX_W, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

        //glTexImage2D
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return texture_id;
    }

    void onError(const char* s) {
        throw std::runtime_error(s);
    }

private:
    std::vector<SubModel> submodels_;

};


/*============================================================================
 *
 * class Figure 
 *
 * 
 *
 *==========================================================================*/
//<<<<<<<<<< Figure

//****************************************************************
// constructor
Figure::Figure() : imp_(new FigureImp) {
}

//****************************************************************
// destructor
Figure::~Figure() {
}

//****************************************************************
// render
void Figure::render() {
    imp_->render();
}

//****************************************************************
// get_transform
Matrix Figure::get_transform() {
    return imp_->get_transform();
}

//****************************************************************
// get_material_color
Color Figure::get_material_color() {
    return imp_->get_material_color();
}

//****************************************************************
// clear
void Figure::clear() {
    imp_->clear();
}

//****************************************************************
// empty
bool Figure::empty() {
    return imp_->empty();
}

//****************************************************************
// build_from_mqo
void Figure::build_from_mqo(
    mqo_reader::document_type& doc, float scale, const Color& color) {
    imp_->build_from_mqo(doc, scale, color);
}

//>>>>>>>>>> Figure

