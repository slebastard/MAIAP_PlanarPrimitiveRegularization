
#ifndef POLY_H
#define POLY_H

#include "types.h"


template <class Refs>
class My_face : public CGAL::HalfedgeDS_face_base<Refs> {
public:
	My_face(){};

inline int& index() { return m_id; }
inline int index() const { return m_id; }
inline int& label() { return m_label; }
inline int label() const { return m_label; }
inline CGAL::Color& color() { return m_color; }
inline CGAL::Color color() const { return m_color; }
inline Vector_Flo& normal() { return m_normal; }
inline Vector_Flo normal() const { return m_normal; }
inline double& z() { return m_z; }
inline double z() const { return m_z; }
inline float& area() { return m_area; }
inline float area() const { return m_area; }
inline Point_Flo& center() { return m_center; }
inline Point_Flo center() const { return m_center; }

private:

int m_id;
int m_label;
CGAL::Color m_color;
Vector_Flo m_normal;
float m_area;
double m_z;
Point_Flo m_center;


};

//definition of a perso vertex for the polyhedron
template <class Refs, class Point>
  class  My_vertex : public CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, Point> {
  public:
	My_vertex() {}
    My_vertex(const Point& pt) : CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, Point>(pt) {}

inline int& index() { return m_id; }
inline int index() const { return m_id; }
inline CGAL::Color& color() { return m_color; }
inline CGAL::Color color() const { return m_color; }
Point_d Pointd(){return  Point_d(point().x(),point().y(),point().z());};
operator const Point_d (){return  Point_d(point().x(),point().y(),point().z());};
operator Point_d (){return  Point_d(point().x(),point().y(),point().z());};


 private:
int m_id;
CGAL::Color m_color;

};

//definition of a perso halfedge for the polyhedron
template <class Kernel, class Refs, class Tprev, class Tvertex, class Tface, class Normal>
class My_halfedge : public CGAL::HalfedgeDS_halfedge_base<Refs,Tprev,Tvertex,Tface>
{
public:
	typedef typename Kernel::FT FT;
	typedef typename Kernel::Point_3 Point_3;
	typedef typename Kernel::Segment_3 Segment_3;

public:
	My_halfedge() { /*m_temp_boolean = false;*/}

public:

	inline int& index() { return m_id; }
	inline int index() const { return m_id; }

	FT length() const
	{
		Segment_3 segment = make_segment();
		return std::sqrt(segment.squared_length());
	}

    Segment_3 make_segment() const
    {
        const Point_3& s = this->opposite()->vertex()->point();
        const Point_3& t = this->vertex()->point();
        return Segment_3(s, t);
    }   

private:
	int m_id;
};

// An items type using my face, halfedge and vertex.
struct My_items : public CGAL::Polyhedron_items_3 {
    template <class Refs, class Traits>
    struct Face_wrapper {
         typedef My_face<Refs> Face;
    };

    template <class Refs, class Traits>
    struct Vertex_wrapper {
        typedef typename Traits::Point_3 Point;
        typedef My_vertex<Refs, Point> Vertex;
    };


	// wrap halfedge
	template<class Refs, class Traits> struct Halfedge_wrapper
	{
		typedef typename Traits::Vector_3 Normal;
		typedef My_halfedge<Traits, Refs, CGAL::Tag_true, CGAL::Tag_true, CGAL::Tag_true, Normal> Halfedge;
	};

};

typedef CGAL::Polyhedron_3<Kernelsc, My_items> Polyhedron_Flo;


typedef Polyhedron_Flo::Vertex Vertex;
typedef Polyhedron_Flo::Vertex_handle Vertex_handle;
typedef Polyhedron_Flo::Vertex_const_handle Vertex_const_handle;
typedef Polyhedron_Flo::Vertex_iterator Vertex_iterator;
typedef Polyhedron_Flo::Vertex_const_iterator Vertex_const_iterator;
typedef Polyhedron_Flo::Halfedge_handle Halfedge_handle;
typedef Polyhedron_Flo::Halfedge_iterator Halfedge_iterator;
typedef Polyhedron_Flo::Edge_iterator Edge_iterator;
typedef Polyhedron_Flo::Facet_handle Facet_handle;
typedef Polyhedron_Flo::Facet_iterator Facet_iterator;
typedef Polyhedron_Flo::Point_iterator Point_iterator;
typedef Polyhedron_Flo::Halfedge_around_vertex_circulator Halfedge_around_vertex_circulator;
typedef Polyhedron_Flo::Halfedge_around_facet_circulator HF_circulator;



#endif