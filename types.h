#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include <list>
#include <map>



#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

#include <CGAL/IO/Color.h>
#include <CGAL/Kd_tree_rectangle.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Point_container.h>
#include <CGAL/Point_2.h>
#include <CGAL/Splitters.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/bounding_box.h>
#include <CGAL/centroid.h>
//#include <CGAL/jet_smooth_point_set.h>
//#include <CGAL/pca_estimate_normals.h> 

//test line-points
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/Segment_3.h>


//convex hull
#include <CGAL/algorithm.h>
#include <CGAL/Convex_hull_traits_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/ch_graham_andrew.h>





//Delaunay 2D + alpha shape 2D
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Polygon_2.h> 
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h> 
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Triangulation_conformer_2.h>
typedef CGAL::Polygon_2<Kernel> Polygon_2;
typedef CGAL::Alpha_shape_vertex_base_2<Kernel>    Vb2; 
typedef CGAL::Alpha_shape_face_base_2<Kernel>    Fb2; 
typedef CGAL::Triangulation_data_structure_2<Vb2,Fb2> Tds2; 
typedef CGAL::Delaunay_triangulation_2<Kernel,Tds2> Triangulation_2; 
typedef CGAL::Alpha_shape_2<Triangulation_2>  Alpha_Shape;
 /*typedef CGAL::Triangulation_vertex_base_2<Kernel> Vb2t;
typedef CGAL::Delaunay_mesh_face_base_2<Kernel> Fb2t;
typedef CGAL::Triangulation_data_structure_2<Vb2t, Fb2t> Tds2t;
typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel, Tds2t, CGAL::Exact_intersections_tag> CDT2;*/
typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel> CDT2;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDT2> Criteria; 
typedef CGAL::Delaunay_mesher_2<CDT2, Criteria> Meshing_engine;



typedef Kernel::Point_3 Point_d;
typedef Kernel::Segment_3 Segment_d;
typedef Kernel::Point_2 Point_2d;

typedef CGAL::Simple_cartesian<double>     Kernelsc;
typedef Kernel::FT FT;
typedef CGAL::Search_traits_3<Kernel> SearchTraits_3;
//typedef CGAL::Search_traits_3<Kernel> SearchTraits_3sc;
typedef CGAL::Search_traits_2<Kernel> SearchTraits_2;
typedef CGAL::Orthogonal_k_neighbor_search<SearchTraits_3> Neighbor_search;
//typedef CGAL::Orthogonal_k_neighbor_search<SearchTraits_3sc> Neighbor_search3sc;
typedef CGAL::Orthogonal_k_neighbor_search<SearchTraits_2> Neighbor_search2;
typedef Neighbor_search::Tree Tree;
typedef Neighbor_search2::Tree Tree2;
//typedef Neighbor_search3sc::Tree Tree3sc;
typedef Kernel::Iso_cuboid_3 bounding_3;
typedef Kernel::Iso_rectangle_2 bounding_2;


//test-convex hull
typedef CGAL::Convex_hull_traits_3<Kernel>             Traits;
typedef Traits::Polyhedron_3                      Polyhedron;
typedef Polyhedron::HalfedgeDS                  HalfedgeDS;

//test line-points
typedef Kernel::Line_3 Line;
typedef Kernel::Plane_3 Plane_3;
typedef Kernel::Vector_3 Vector_3;

typedef Kernel::Segment_2 Segment_2d;
typedef Kernel::Line_2 Line_2;




typedef CGAL::Simple_cartesian<double>     Kernelsc;
typedef Kernelsc::Plane_3 Plane_Flo;
typedef Kernelsc::Vector_3 Vector_Flo;
typedef Kernelsc::Point_3 Point_Flo;
typedef Kernelsc::Segment_3 Segment_Flo;
typedef Kernelsc::Vector_2 Vector_Flo_2;
typedef Kernelsc::Line_3 Line_Flo;
typedef Kernelsc::Plane_3 Plane;


#define PARAM_OFSTREAM_TEXTE std::ios::out
#define OFSTREAM_TEXTE(nomvar, nomfic) std::ofstream nomvar (nomfic, PARAM_OFSTREAM_TEXTE) ; nomvar.setf(std::ios::fixed);

