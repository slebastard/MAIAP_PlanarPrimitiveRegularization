#ifndef CLOUD_SEGMENTATION_H
#define CLOUD_SEGMENTATION_H

#include "types.h"
#include "Ply.hpp"
#include "Visualization_Tools.h"
#include "Polyhedron_Flo.h"
#include "GCoptimization.h"
#include <cassert>
#include <time.h>
#include <CGAL/Polytope_distance_d.h>

typedef CGAL::Polytope_distance_d<Traits>     Polytope_distance;

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::set;


using I::misc::Ply;
typedef unsigned char byte;
const double PI = 3.141592;

double distance_point_d(Point_d p1,Point_d p2){return sqrt(pow(p1.x()-p2.x(),2)+pow(p1.y()-p2.y(),2)+pow(p1.z()-p2.z(),2));}

   //--------------------------//
  //------SUBPLANE-CLASS------//
 //--FOR-PRIMITIVE-MERGING---//
//---------------------------//

class subPlane
{
protected:
	Plane Splane;
	vector<Facet_handle> Sfacets;
	Facet_handle Sseed;
	float Sarea;
public:
	/**
	* Default subPlane constructor
	* @return new subPlane
	*/
	subPlane()
	{
	};

	/**
	* Default subPlane constructor
	* @param P plane to be attached to this subPlane. This plane should represent in some way the position of the subPlane in space
	* @param fcts a vector of facets to be attached to the subPlane
	* @param sd a seed facet to be attributed to the subPlane
	* @param area 
	* @return new subPlane
	*/
	subPlane(const Plane &P, const vector<Facet_handle> &fcts, const Facet_handle &sd)
	{
		Splane = P;
		Sfacets = fcts;
		Sseed = sd;
		Sarea = 0;
		for (vector<Facet_handle>::iterator fit = Sfacets.begin(); fit != Sfacets.end(); fit++)
			Sarea += (*fit)->area();
	};
	/**
	* Plane get method
	* @return plane representent of subPlane
	*/
	Plane plane(){ return Splane; };
	/**
	* Facets get method
	* @return vector of facets handles for this subPlane
	*/
	vector<Facet_handle> facets(){ return Sfacets; };
	/**
	* Seed facet get method
	* @return seed facet handle for this subPlane
	*/
	Facet_handle seed(){ return Sseed; };
	/**
	* Area get method
	* @return area for this subPlane
	*/
	float area(){ return Sarea; };
	/**
	* Plane set method
	* @param plane to be attributed to the subPlane
	*/
	void set_plane(const Plane &p){ Splane=p; };
	/**
	* Facets set method
	* @param vector of facet handles to be attributed to the subPlane
	*/
	void set_facets(const vector<Facet_handle> &f){
		Sfacets = f;
		Sarea = 0;
		for (vector<Facet_handle>::iterator fit = Sfacets.begin(); fit != Sfacets.end(); fit++)
			Sarea += (*fit)->area();
	};
	/**
	* Seed set method
	* @param facet handle for the seed to be attributed to the subPlane
	*/
	void set_seed(const Facet_handle &sd){ Sseed = sd; };

	/**
	* Indicates whether this.plane() and other_plane.plane() are parallel, with a angular precision of angle_max
	* @param other_plane
	* @param angle_max
	* @return true if planes are considered parallel, false otherwise
	*/
	bool is_parallel_to(subPlane other_plane, float angle_max)
	{
		if (abs(plane().orthogonal_vector()*other_plane.plane().orthogonal_vector()) > cos(angle_max))
			return true;
		return false;
	};

	/**
	* Indicates whether this.plane() and other_plane.plane() are coplanar
	* @param angle_max is the maximum angular rotation between the two plans
	* @param angle_max_copl if S1 and S2 are the two seeds, is the maximum angle between vector S1S2 and the primitive's noraml vectors
	* @return true if planes are considered coplanar, false otherwise
	*/
	bool is_coplanar_to(subPlane other_plane, float angle_max, float angle_max_copl = 0.8)
	{
		Vector_Flo dir(seed()->halfedge()->vertex()->point(), other_plane.seed()->halfedge()->vertex()->point());
		dir = dir / sqrt(dir*dir);
		if (is_parallel_to(other_plane, angle_max) && (abs(dir * seed()->normal()) < 1 - cos(angle_max_copl)) && (abs(dir * other_plane.seed()->normal()) < 1 - cos(angle_max_copl)))
			return true;
		return false;
	};

	/**
	* Indicates whether this.plane() and other_plane.plane() can be considered orthogonal
	* with a angular precision of angle_max
	* @param other_plane is the fellow subPlane to be considered
	* @param angle_max represents (PI/2 - maximum angular rotation between the two plans)
	* @return true if planes are considered orthogonal, false otherwise
	*/
	bool is_orthogonal_to(subPlane other_plane, float angle_max)
	{
		if (abs(plane().orthogonal_vector()*other_plane.plane().orthogonal_vector()) < cos(M_PI / 2 - angle_max))
			return true;
		return false;
	};

	/**
	* Indicates whether this.plane() and other_plane.plane() can be considered adjacent
	* The two seeds must be closed enough to eachother, relatively to the areas of the subplanes
	* @param other_plane is the fellow subPlane to be considered
	* @param threshold a metric object to determine whether the two subPlanes are adjacent
	* @param angle_max a angular object to determine whether the two subPlanes are adjacent
	* @return true if planes are considered tp be adjacent, false otherwise
	*/
	bool is_adjacent_to(subPlane other_plane, float threshold, float angle_max)
	{
		/*Polytope_distance subplanes_distance(facets().begin(), facets().end(), other_plane.facets().begin(), other_plane.facets().end());
		float dist = sqrt(subplanes_distance.squared_distance());
		if (is_stackable_to(other_plane) && dist / (area() + other_plane.area()) < threshold)
			return true;
			*/
		return false;
	}
};

class Cloud_segmentation
{
public:
Polyhedron_Flo mesh;
double zmin;
double zmax;
double zmoy;
vector<vector<Facet_handle> > list_neighborg_facet;
vector<Facet_iterator> list_facets;
vector<subPlane> extracted_planes;

bool save_result(){

	char *nom_mesh= "result_classification.ply";
	save_polyhedron2ply(mesh,nom_mesh,0);

	return true;
}

bool save_result2(){

	char *nom_mesh = "result_plane_extraction.ply";
	save_polyhedron2ply(mesh, nom_mesh, 0);

	return true;
}	

bool Cloud_segmentation::init()
{
	int count = 0;
	zmin = 1e7;
	zmax = -1e7;
	zmoy = 1e7;
	for (Polyhedron_Flo::Vertex_iterator vi = mesh.vertices_begin(); vi != mesh.vertices_end(); ++vi, ++count){
		vi->index() = count;
		double z = vi->point().z();
		zmoy += z;
		if (zmin > z) zmin = z;
		if (zmax < z) zmax = z;
	}
	zmoy /= mesh.size_of_vertices();

	//computation FACET normals
	count = 0;
	list_facets = vector<Facet_iterator>(mesh.size_of_facets());

	for (Facet_iterator f = mesh.facets_begin(); f != mesh.facets_end(); ++f, ++count)
	{
		list_facets[count] = (f);
		Halfedge_handle
			h1 = f->halfedge(),
			h2 = h1->next(),
			h3 = h2->next();

		//calcul des normals
		Polyhedron_Flo::Vertex_handle
			v1 = h1->vertex(),
			v2 = h2->vertex(),
			v3 = h3->vertex();

		f->center() = Point_Flo(1 / 3 * (v1->point().x() + v2->point().x() + v3->point().x()), 1 / 3 * (v1->point().y() + v2->point().y() + v3->point().y()), 1 / 3 * (v1->point().z() + v2->point().z() + v3->point().z()));
		f->normal() = CGAL::orthogonal_vector(v1->point(), v2->point(), v3->point()) / float(2);
		f->area() = std::max<float>(CGAL::sqrt(f->normal() * f->normal()), 1e-7);
		f->normal() = f->normal() / f->area();
		f->index() = count;
		f->z() = (v1->point().z() + v2->point().z() + v3->point().z()) / 3;
	}


	//Compute neighboring facets
	list_neighborg_facet = vector<vector<Facet_handle> >(mesh.size_of_facets());

	for (int i = 0; i < mesh.size_of_facets(); i++){
		Facet_iterator f = list_facets[i];
		//get neighbors
		Halfedge_handle
			h1 = f->halfedge(),
			h2 = h1->next(),
			h3 = h2->next();

		//calcul des normals
		vector<Polyhedron_Flo::Vertex_handle> vert(3);
		vert[0] = h1->vertex(); vert[1] = h2->vertex(); vert[2] = h3->vertex();

		vector<int> index_facet_neighborg;
		set<int> index_neighborg_facet;

		bool  is_max_local = false;
		bool  is_min_local = false;
		Halfedge_around_vertex_circulator h0, h;
		for (int i = 0; i < vert.size(); i++)
		{
			h0 = vert[i]->vertex_begin(); h = h0;
			do {
				Facet_handle f1 = h->facet();
				Facet_handle f2 = h->opposite()->facet();
				if (!h->is_border_edge() && f != f1 && f != f2)
				{
					index_neighborg_facet.insert(f1->index());
					index_neighborg_facet.insert(f2->index());
				}
			} while (++h != h0);
		}

		if (!h1->is_border_edge())
			list_neighborg_facet[i].push_back(h1->opposite()->facet());

		if (!h2->is_border_edge())
			list_neighborg_facet[i].push_back(h2->opposite()->facet());

		if (!h3->is_border_edge())
			list_neighborg_facet[i].push_back(h3->opposite()->facet());

		//then the remaining
		for (set<int>::iterator it = index_neighborg_facet.begin(); it != index_neighborg_facet.end(); it++)
		{
			list_neighborg_facet[i].push_back(list_facets[(*it)]);
		}
	}

	return true;
}

  //--------------------------------//
 //------FEATURES-COMPUTATION------//
//--------------------------------//

/**
* Polynomial function for ground confidence function
* @param z is the height
*/
float Cloud_segmentation::ground_poly(float z)
{
	return (exp(zmin - z) - exp(zmin - zmax)) / (1 - exp(zmin - zmax));
}

// Confidence for ground
/**
* Confidence function for ground
* @param f is the facet we must assess the ground confidence of
* @param z_dir a vector that should ideally be normal to the ground in this region
* @param hweight ponderation factor for height component
* @param nweight ponderation factor for orientation component
* @return ground confidence level (0: no confidence, 1: high confidence)
*/
float Cloud_segmentation::ground_confidence(Facet_handle f, Vector_Flo z_dir, float hweight = 1, float nweight = 0.8)
{
	float height_score = ground_poly(f->z());
	float normal_score = abs((f->normal())*z_dir);
	float ground_score = std::pow(height_score, hweight)*std::pow(normal_score, nweight);
	return ground_score;
}

/**
* Height exponential function for roof confidence function
* @param z is the height
*/
float Cloud_segmentation::roof_exp(float z)
{
	float zmin_roof = (float(39) / float(40))*zmin + (float(1)/ float(40))*zmoy;
	if (z > zmin_roof)
		return (exp(zmax - z) - exp(zmax - zmin_roof)) / (1 - exp(zmax - zmin_roof));
	else
		return 0;
}

/**
* Confidence function for roof
* @param f is the facet we must assess the roof confidence of
* @param z_dir a vertical vector
* @param hweight ponderation factor for height component
* @param nweight ponderation factor for orientation component
* @return roof confidence level (0: no confidence, 1: high confidence)
*/
float Cloud_segmentation::roof_confidence(Facet_handle f, Vector_Flo z_dir, float hweight = 1.2, float nweight = 1)
{
	float height_score = roof_exp(f->z());
	float normal_score_inf = abs(f->normal()*z_dir);
	Vector_Flo roof_attr = cross_product(f->normal(), z_dir);
	float normal_score_sup = abs(roof_attr*roof_attr);
	float normal_score = 2 * normal_score_inf*normal_score_sup;
	float roof_score = std::pow(height_score, hweight)*std::pow(normal_score, nweight);
	return roof_score;
}

/**
* Polynomial function for facade confidence function
* @param z is the height
*/
float Cloud_segmentation::facade_poly(float z)
{
	float zmin_facade = (float(299) / float(300))*zmin + (float(1) / float(300))*zmoy;
	float gamma = std::pow(2 / (zmax - zmin_facade), 2);
	if (z > zmin_facade)
		return gamma*(z - zmin_facade)*(zmax - z);
	else
		return 0;
}

/**
* Confidence function for facade
* @param f is the facet we must assess the roof confidence of
* @param z_dir a vertical vector
* @param hweight ponderation factor for height component
* @param nweight ponderation factor for orientation component
* @return facade confidence level (0: no confidence, 1: high confidence)
*/
float Cloud_segmentation::facade_confidence(Facet_handle f, Vector_Flo z_dir, float hweight = 0.25, float nweight = 1)
{
	float height_score = (facade_poly(f->z()));
	Vector_Flo facade_attr = cross_product(f->normal(), z_dir);
	float normal_score = facade_attr*facade_attr;
	float facade_score = pow(height_score, hweight)*pow(normal_score, nweight);
	return facade_score;
}

/**
* Classification of the facets of the mesh into ground, facade, roof or none of the three
* @param beta penalty on facet area for boundaries between two classes
* @param gamma penalty on edge length for boundaries between two classes
* @return state variable
*/
bool Cloud_segmentation::labeling(double beta, double gamma)
{
	cout << "--------------------------------------------------" << endl;
	cout << "- Classification du maillage en sol/facade/toit -" << endl;
	cout << "--------------------------------------------------" << endl << endl;
	Vector_Flo vz(0., 0., 1.);
	srand(time(NULL));
	//Graph creation
	//------------------------------------------------------------------------
	int number_of_class = 3 ;
	int number_nodes = mesh.size_of_facets();
	GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(number_nodes,number_of_class);
	//------------------------------------------------------------------------

	//Graph adjacency setup
	//------------------------------------------------------------------------
	for (Facet_iterator f = mesh.facets_begin() ; f != mesh.facets_end() ; ++f)
	{
		int index_facet=f->index();
		// ---------------------------------------------------------------------------------------------
		// Q2) Prise en compte de l'aire de la facette et de la longueur des arêtes dans la pénalisation
		//----------------------------------------------------------------------------------------------
		// On choisit ici une pénalisation quadratique sur l'aire de la facette
		float weight=beta*pow(f->area(),2);

		Halfedge_handle
			h1 = f->halfedge(),
			h2 = f->halfedge()->next(),
			h3 = f->halfedge()->next()->next();

		float
			l1 = h1->length(),
			l2 = h2->length(),
			l3 = h3->length();

		// ---------------------------------------
		// Spécifie quelles facettes sont voisines
		// ---------------------------------------
		if ( !h1->is_border_edge() ){
			int f1_index=h1->opposite()->facet()->index();
			float w = weight*pow(h1->opposite()->facet()->area(), 2);
			gc->setNeighbors(index_facet, f1_index, w + gamma*h1->length());
		}

		if ( !h2->is_border_edge() ){
			int f2_index=h2->opposite()->facet()->index();
			float w = weight*pow(h2->opposite()->facet()->area(), 2);
			gc->setNeighbors(index_facet, f2_index, w + gamma*h2->length());
		}

		if ( !h3->is_border_edge() ){
			int f3_index=h3->opposite()->facet()->index();
			float w = weight*pow(h3->opposite()->facet()->area(), 2);
			gc->setNeighbors(index_facet, f3_index, w + gamma*h3->length());
		}
	}

	float *data = new float[number_nodes*number_of_class];

	for (Facet_iterator f = mesh.facets_begin() ; f != mesh.facets_end() ; ++f)
	{
		int index_facet=f->index();
		
		// -------------------------------------------------------------------
		// Attribution des confiances pour cette facette et pour chaque classe
		// -------------------------------------------------------------------
		data[index_facet*number_of_class + 0] = 1 - ground_confidence(f, vz);
		data[index_facet*number_of_class + 1] = 1 - roof_confidence(f, vz);
		data[index_facet*number_of_class + 2] = 1 - facade_confidence(f, vz);
	}
	gc->setDataCost(data);
	//------------------------------------------------------------------------

	//Potential (pairwise interaction)  -> TO FILL IN
	//------------------------------------------------------------------------
	float *smooth = new float[number_of_class*number_of_class];
	for ( int l1 = 0; l1 < number_of_class; l1++ )
	{
		for (int l2 = 0; l2 < number_of_class; l2++)
		{
			smooth[l1 + l2*number_of_class] = 0;
			if (l1 != l2)
			{
				smooth[l1 + l2*number_of_class] = 1;
			}
		}
	}

	gc->setSmoothCost(smooth);
	//------------------------------------------------------------------------

	//Alpha-expansion
	//------------------------------------------------------------------------
	for (int i = 0; i < 2; i++)
		for (int next = 0; next < number_of_class; next++){
			gc->alpha_expansion(next);
			cout<<"Cycle "<<i+1<<" : expansion label "<<next+1<<endl;
		}

	//------------------------------------------------------------------------
	//Save results and clear structures
	//------------------------------------------------------------------------
	for (Facet_iterator f = mesh.facets_begin() ; f != mesh.facets_end() ; ++f){
		
		int index_facet=f->index();
		f->label() = gc->whatLabel(index_facet); 

		if(f->label()==0) f->color()=CGAL::Color(255,255,150);
		else if(f->label()==1) f->color()=CGAL::Color(120,127,250);
		else f->color()=CGAL::Color(250,230,230);
	}

	delete gc;
	delete [] smooth;
	delete [] data;
	//------------------------------------------------------------------------

	return true;
}

/**
* Predicate function: are the facets list_facets[i] and list_facets[j] parallel?
* Solely used for the NORMALS primitive detection method
* @param i index of the first facet
* @param j index of the second facet
* @param max_angle maximum angle (rad) between the normal vectors of facets for them to be parallel
* @return true if list_facets[i] and list_facets[j] can be considered parallel, false otherwise
*/
bool satisfies_parallel_propagation_condition(int i, int j, float max_angle)
{
	assert(0 <= i && i < list_facets.size() && 0 <= j && j < list_facets.size());
	Vector_Flo normal_branch = list_facets[i]->normal();
	Vector_Flo normal_leaf = list_facets[j]->normal();
	if (normal_branch*normal_leaf > cos(max_angle))
		return true;
	return false;
}

/**
* Predicate function: is list_facets[i] orthogonal to vector n?
* Solely used for the DYN_NORMALS primitive detection method, to know if facet list_facets[i] will
* integrate primitive of average normal vector estimation n
* @param i index of the candidate facet
* @param n current estimation of the vector normal to the primitive
* @param max_angle maximum angle (rad) between the normal vector of candidate facet and n for the facet to be accepted
* @return true if list_facets[i]  can be considered orthogonal to n, false otherwise
*/
bool satisfies_parallel_propagation_condition_dynamic(int i, Vector_Flo n, float max_angle)
{
	assert(0 <= i && i < list_facets.size() && 0 <= j && j < list_facets.size());
	Vector_Flo normal_branch = list_facets[i]->normal();
	if (normal_branch*n > cos(max_angle))
		return true;
	return false;
}

// -------------------------------------------------------------------------------------------------------------------
// Predicat: existe-t-il un plan optimal_plane dont la distance moyenne aux facettes de class_set soit assez petite?
// --------------------------------------------------------------------------------------------------------------------
/**
* Predicate function: is there a plane which creates an average distance to the facets of primitive low enough?
* Solely used for the PRIMFIT & PRIMFIT2 primitive detection method
* @param class_set set of Facet_handle of the current facets inside the primitive, + the candidate facet
* @param optimal_plane plane variable to be affected with the best fitting plane there can be
* @param max_angle cos(max_angle) corresponds to the maximal approximation error of the facets set by the plane
* @param scale space parameter to take into account the area or number of facets inside the primitive
* @return true if the plane that minimizes this distance satisfies the previous condition, false otherwise
*/
bool satisfies_primitive_fitting_condition(std::vector<Facet_handle> class_set, Plane &optimal_plane, float max_angle = 0.09, float scale=0)
{
	std::vector<Point_Flo> centroid_set;
	int k = 0;
	for (std::vector<Facet_handle>::iterator face_it = class_set.begin(); face_it != class_set.end(); face_it++)
	{
		Point_Flo p1 = (*face_it)->halfedge()->vertex()->point();
		Point_Flo p2 = (*face_it)->halfedge()->next()->vertex()->point();
		Point_Flo p3 = (*face_it)->halfedge()->next()->next()->vertex()->point();
		Point_Flo centro = centroid(p1, p2, p3);
		centroid_set.push_back(centro);
		k++;
	}
	float fitting_quality = CGAL::linear_least_squares_fitting_3(centroid_set.begin(), centroid_set.end(), optimal_plane, CGAL::Dimension_tag<0>());
	if (1 - fitting_quality < (1 - cos(max_angle))*exp(-scale*(k - 1)))
		return true;
	return false;
}

/**
* Predicate macrofunction: calls on:
* - satisfies_parallel_propagation_condition for NORMALS
* - satisfies_parallel_propagation_condition_dynamic for DYN_NORMALS
* - satisfies_primitive_fitting_condition for PRIMFIT & PRIMFIT2
* @param point_index reference facet inside primitive
* @param neighbor_index candidate facet
* @param class_set set of Facet_handle of the current facets inside the primitive, + the candidate facet
* @param optimal_plane plane variable to be affected with the best fitting plane there can be
* @param n_v current estimation of the best normal vector to the primitive
* @param method method of belief propagation, among: NORMALS (compare facets normals to seed normal), DYN_NORMALS (default: compare the facet normal to an estimation of the primitive normal), PRIMFIT (try to fit a plane to the primitive + candidate facet)
* @param max_angle cos(max_angle) corresponds to the maximal approximation error of the facets set by the plane
* @param scale space parameter to take into account the area or number of facets inside the primitive
* @return true if the candidate facet satisfies the condition to integrate the primitive
*/
bool satisfies_growing_condition(int point_index, int neighbor_index, std::vector<Facet_handle> class_set, Plane& optimal_plane, Vector_Flo n_v, string method = "NORMALS", float max_angle = 0.09, float scale = 0)
{
	if (method == "PRIMFIT")
		return satisfies_primitive_fitting_condition(class_set, optimal_plane, max_angle);
	else if (method == "PRIMFIT2")
		return satisfies_primitive_fitting_condition(class_set, optimal_plane, max_angle, scale);
	else if (method == "NORMALS")
		return satisfies_parallel_propagation_condition(point_index, neighbor_index, max_angle);
	else
		return satisfies_parallel_propagation_condition_dynamic(neighbor_index, n_v, max_angle);
}

/**
* Determines a score between 0 and 1 about how the candidate facet fits to the primitive. Fuzzy version of:
* - satisfies_parallel_propagation_condition for NORMALS
* - satisfies_parallel_propagation_condition_dynamic for DYN_NORMALS
* - satisfies_primitive_fitting_condition for PRIMFIT & PRIMFIT2
* @param point_index reference facet inside primitive
* @param neighbor_index candidate facet
* @param class_set set of Facet_handle of the current facets inside the primitive, + the candidate facet
* @param optimal_plane plane variable to be affected with the best fitting plane there can be
* @param n_v current estimation of the best normal vector to the primitive
* @param method method of belief propagation, among: NORMALS (compare facets normals to seed normal), DYN_NORMALS (compare the facet normal to an estimation of the primitive normal), PRIMFIT (try to fit a plane to the primitive + candidate facet)
* @param max_angle cos(max_angle) corresponds to the maximal approximation error of the facets set by the plane
* @param scale space parameter to take into account the area or number of facets inside the primitive
* @return a float between 0 and 1 reflecting the likelihood of the candidate facet belonging to the primitive
*/
float fitting_score(int point_index, int neighbor_index, std::vector<Facet_handle> class_set, Plane& optimal_plane, Vector_Flo n_v, string method = "NORMALS", float max_angle = 0.09, float scale = 0)
{
	if (method == "PRIMFIT" || method == "PRIMFIT2")
	{
		std::vector<Point_Flo> centroid_set;
		int k = 0;
		for (std::vector<Facet_handle>::iterator face_it = class_set.begin(); face_it != class_set.end(); face_it++)
		{
			Point_Flo p1 = (*face_it)->halfedge()->vertex()->point();
			Point_Flo p2 = (*face_it)->halfedge()->next()->vertex()->point();
			Point_Flo p3 = (*face_it)->halfedge()->next()->next()->vertex()->point();
			Point_Flo centro = centroid(p1, p2, p3);
			centroid_set.push_back(centro);
			k++;
		}
		return(CGAL::linear_least_squares_fitting_3(centroid_set.begin(), centroid_set.end(), optimal_plane, CGAL::Dimension_tag<0>()));
	}
	assert(0 <= point_index && point_index < list_facets.size() && 0 <= neighbor_index && neighbor_index < list_facets.size());
	if (method == "NORMALS")
	{
		Vector_Flo normal_branch = list_facets[point_index]->normal();
		Vector_Flo normal_leaf = list_facets[neighbor_index]->normal();
		return(normal_branch*normal_leaf);
	}
	else
	{
		Vector_Flo normal_leaf = list_facets[neighbor_index]->normal();
		return(normal_leaf*n_v);
	}
}

/**
* Returns a vector of facets from a vector of indexes
* @param indexes vector of facet indexes
* @return vector of facets corresponding to the input indexes
*/
vector<Facet_handle> facet_list(vector<int> indexes)
{
	vector<Facet_handle> facets;
	for (vector<int>::iterator i = indexes.begin(); i != indexes.end(); i++)
		facets.push_back(list_facets[*i]);
	return facets;
}

/**
* Predicate: when list_facets[facet_index] belongs to no primitive, is it a stuck between primitives (true) or is it free (false)?
* @param facet_index index of observed facet
* @param class_index the primitive class of observation
* @param label_region maps facets to primitives (-1 for no primitive)
* @return true if the facet is "surrendered" (2 pr more neighbors belonging to primitive), false otherwise
*/
bool insider(int facet_index, int class_index, std::vector<int> label_region)
{
	Facet_iterator f = list_facets[facet_index];
	Halfedge_handle h1 = f->halfedge();
	Halfedge_handle h2 = f->halfedge()->next();
	Halfedge_handle h3 = f->halfedge()->next()->next();
	int k1 = 0, k2 = 0, k3 = 0;
	if (!h1->is_border_edge())
		k1 = label_region[h1->opposite()->facet()->index()];
	if (!h2->is_border_edge())
		k2 = label_region[h2->opposite()->facet()->index()];
	if (!h3->is_border_edge())
		k3 = label_region[h3->opposite()->facet()->index()];
	int nb_insider_neighbors = int(k1 == class_index) + int(k2 == class_index) + int(k3 == class_index);
	return (nb_insider_neighbors > 1);
}

/**
* Returns the centroid of a facet
* @param f facet_handle to observed facet
* @return centroid of the facet
*/
Point_Flo facet_centroid(Facet_handle f)
{
	Point_Flo p1 = f->halfedge()->vertex()->point();
	Point_Flo p2 = f->halfedge()->next()->vertex()->point();
	Point_Flo p3 = f->halfedge()->next()->next()->vertex()->point();
	return centroid(p1, p2, p3);
}

/**
* Explored the mesh and generates planar primitives, with region growing according to the selected method:
* - NORMALS: propagate by comparing the normal vector of the candidate facet to the normal vector of the seed
* - DYN_NORMALS: keeps updated an estimation of the most accurate normal vector to the primitive, and compares the normal vector of the candidate facet to it
* - PRIMFIT: takes into account the current facets of the primitive + the candidate facet, tries to fit a plane to the whole, and integrates the candidates facets is the error is low enough
* - PRIMFIT2: same as PRIMFIT but taking into account the size of the current primitive
* @param angle_max tolerance angle for region growing
* @param angle_max_merge tolerance angle for primitive merging
* @param area_min minimum area of a planar primitive
* @param scale space parameter for PRIMFIT2 method
* @param method method of belief propagation, among: NORMALS (compare facets normals to seed normal), DYN_NORMALS (compare the facet normal to an estimation of the primitive normal), PRIMFIT (try to fit a plane to the primitive + candidate facet)
* @param cluster_mode primitive merging method, among: NONE (no primitive merging), PARA (will merge parallel primitive, with an angular tolerance angle_max_merge), COPL (will merge coplanar primitive), PARA_ORTH (will merge parallel and orthogonal primitives)
* @param nb_min minimal amount of facets inside a primitive
* @param regularize activates primitive regularization (projects vertices on estimated plane)
*/
void Cloud_segmentation::plane_extraction(float angle_max, float angle_max_merge, float area_min, float scale, string method, string cluster_mode, float nb_min, bool regularize)
{
	time_t begin_timer;
	time_t end_timer;
	time_t interm_timer;
	time(&begin_timer);
	double seconds;
	float ang_norm = 1;

	cout << "--------------------------------------------------" << endl;
	cout << "Extraction de primitives par croissance de region" << endl;
	cout << "--------------------------------------------------" << endl << endl;

	std::vector<int> label_region(mesh.size_of_facets(), -1);
	vector<int> fitting_quality(mesh.size_of_facets(), 0);
	int class_index = -1;

	for (Facet_iterator f = mesh.facets_begin(); f != mesh.facets_end(); ++f)
	{
		progress_verbose(f->index(), mesh.size_of_facets());
		if (label_region[f->index()] == -1)
		{
			class_index++;
			time(&interm_timer);
			label_region[f->index()] = class_index;
			double area = f->area();

			//characteristics of the seed
			Vector_Flo normal_seed = f->normal();
			Point_Flo pt_seed = facet_centroid(f);
			Plane optimal_plane(pt_seed, normal_seed);
			Vector_Flo normal_vector = normal_seed;

			//initialization containers
			std::vector < int > index_container; index_container.push_back(f->index());
			std::vector < int > index_container_former_ring; index_container_former_ring.push_back(f->index());
			std::list < int > index_container_current_ring;
			
			//propagation
			bool propagation = true;
			do{
				propagation = false;
				std::vector<Facet_handle> tmp_support;
				for (int k = 0; k < (int)index_container_former_ring.size(); k++){
					int point_index = index_container_former_ring[k];
					Facet_handle former_face = list_facets[point_index];
					tmp_support.push_back(former_face);
				}

				for (int k = 0; k<(int)index_container_former_ring.size(); k++){

					int point_index = index_container_former_ring[k];
					int count = 0;
						
					for (vector<Facet_handle>::iterator it = list_neighborg_facet[point_index].begin(); it != list_neighborg_facet[point_index].end() && count < 3; it++, count++)
					{
						Vector_Flo n1 = list_facets[point_index]->normal();
						Vector_Flo n2 = (*it)->normal();
						int neighbor_index = (*it)->index();
						Facet_handle neighbor_face = list_facets[neighbor_index];
						tmp_support.push_back(neighbor_face);
						// Avec un timer, il est possible de donner de l'importance a la vitesse de l'algorithme au detriment de la precision de l'algorithme
						time(&end_timer);
						seconds = difftime(end_timer, interm_timer);
						if (seconds > 3)
							break;
						// PROBLEME: Avec les methodes PRIMFIT et PRIMFIT2, des subPlanes dont l'aire est inferieure a l'aire demandee sont selectionnees! Pourquoi?
						if (label_region[neighbor_index] != class_index && (satisfies_growing_condition(point_index, neighbor_index, tmp_support, optimal_plane, normal_vector, method, angle_max, scale) || insider(neighbor_index, class_index, label_region)))
						{					
							//std::vector<int>::iterator found_it;
							//found_it = find(index_container.begin(), index_container.end(), neighbor_index);
							//if (found_it == index_container.end())
							float score = fitting_score(point_index, neighbor_index, tmp_support, optimal_plane, normal_vector, method, angle_max, scale);
							// La facette est attribuee a ce groupe seulement si le groupe auquel la facette etait deja attribuee est un "moins bon candidat" au sens d'un score donne
							if (fitting_quality[neighbor_index] < score)
							{
								index_container_current_ring.push_back(neighbor_index);
								propagation = true;
								label_region[neighbor_index] = class_index;
								fitting_quality[neighbor_index] = score;
							}	
						}
						tmp_support.pop_back();
					}
				}

				//update containers
				index_container_former_ring.clear();
				for (std::list < int >::iterator it = index_container_current_ring.begin(); it != index_container_current_ring.end(); ++it){
					index_container_former_ring.push_back(*it);
					index_container.push_back(*it);
				}
				index_container_current_ring.clear();
				normal_vector = Vector_Flo(pt_seed, pt_seed);
				for (std::vector<int>::iterator it = index_container.begin(); it != index_container.end(); ++it){
					normal_vector = normal_vector + list_facets[*it]->normal();
				}
				normal_vector = normal_vector / float(index_container.size());
				optimal_plane = Plane(pt_seed, normal_vector);

			} while (propagation);

			for (std::vector<int>::iterator it = index_container.begin(); it != index_container.end(); ++it){
				area += list_facets[*it]->area();
			}

			//Test the number of inliers -> reject if inferior to Nmin
			//if (area < area_min && index_container.size() < nb_min) {
			if (index_container.size() < nb_min) {
				class_index--;
				label_region[f->index()] = -1;
				for (int k = 0; k < (int)index_container.size(); k++) label_region[index_container[k]] = -1;
			}
			else
			{
				subPlane a(optimal_plane, facet_list(index_container), f);
				extracted_planes.push_back(a);
			}
		}
	}
	time(&end_timer);
	seconds = difftime(end_timer, begin_timer);
	cout << class_index + 1 << " planes detected" << endl;
	cout << "Computation time for at least " << area_min << " facets per primitive and maximum angle of coplanarity of " << angle_max << "degrees : " << seconds << "s." << endl << endl;

	if (class_index == -1)
		return;

	//grouping parallel and orthogonal planes
	std::vector<int> supergroups = merge_primitive(angle_max_merge, cluster_mode);
	int nb_superplanes = *std::max_element(supergroups.begin(), supergroups.end());
	cout << nb_superplanes + 1 << " superplanes detected" << endl;

	if (regularize)
		rectify(label_region);

	//display
	std::vector<CGAL::Color> list_colors;
	//for (int i = 0; i < class_index + 1; i++){
	for (int i = 0; i < nb_superplanes + 1; i++){
		int red = (int)floor((double)(186) * double(rand()) / double(RAND_MAX)) + 50;
		int green = (int)floor((double)(196) * double(rand()) / double(RAND_MAX)) + 40;
		int blue = (int)floor((double)(166) * double(rand()) / double(RAND_MAX)) + 70;
		CGAL::Color col(red, green, blue);
		list_colors.push_back(col);
	}

	for (Facet_iterator f = mesh.facets_begin(); f != mesh.facets_end(); ++f){

		int index_facet = f->index();

		if (label_region[index_facet] == -1) f->color() = CGAL::WHITE;
		//else f->color() = list_colors[label_region[index_facet]];
		else
		{
			if (class_index>=0)f->color() = list_colors[supergroups[label_region[index_facet]]];
			else f->color() = CGAL::WHITE;
		}

	}

}

/**
* Projects the vertices of planar primitives on their respective planes
* @param label_region maps facets to planar primitives (-1 if facet is in no primitive)
*/
void rectify(std::vector<int> label_region)
{
	for (Facet_iterator f = mesh.facets_begin(); f != mesh.facets_end(); ++f)
	{
		Halfedge_handle h1 = f->halfedge();
		Halfedge_handle h2 = f->halfedge()->next();
		Halfedge_handle h3 = f->halfedge()->next()->next();
		Vertex_handle v1 = h1->vertex();
		Vertex_handle v2 = h2->vertex();
		Vertex_handle v3 = h3->vertex();
		int region_class = label_region[f->index()];
		if (region_class != -1)
		{
		/*  Halfedge_around_vertex_circulator circ = v1->vertex_begin();
			Halfedge_around_vertex_circulator end = circ;
			vector<Vertex> adjacent_vertices;
			CGAL_For_all(circ, end)
			{
				adjacent_vertices.push_back((*circ)->opposite()->vertex());
				erase_facet(*circ);
			} */
			Point_Flo& p1 = v1->point();
			Point_Flo& p2 = v2->point();
			Point_Flo& p3 = v3->point();
			p1 = extracted_planes[region_class].plane().projection(v1->point());
			p2 = extracted_planes[region_class].plane().projection(v2->point());
			p3 = extracted_planes[region_class].plane().projection(v3->point());
		}
	}
}

/**
* Aligns all coplanar primitives
* @param label_superregion maps primitives to their group of primitive
*/
void align(std::vector<int> label_superregion)
{
	int max = *std::max_element(label_superregion.begin(), label_superregion.end());
	for (int i = 0; i<max; i++)
	{
		double a, b, c, d = 0;
		int count = 0;
		for (int j = 0; j<extracted_planes.size(); ++j)
		{
			if (label_superregion[j] != i)
				continue;
			Plane plan = extracted_planes[j].plane();
			a += plan.a();
			b += plan.b();
			c += plan.c();
			d += plan.d();
			count++;
		}
		a = a / double(count);
		b = b / double(count);
		c = c / double(count);
		d = d / double(count);
		Plane p = Plane(a, b, c, d);
		for (int j = 0; j < extracted_planes.size(); j++)
		{
			if (label_superregion[j] != i)
				continue;
			extracted_planes[j].set_plane(p);
		}
			
	}
}

/**
* Aligns all parallel primitives
* @param label_superregion maps primitives to their group of primitive
*/
void rotate(std::vector<int> label_superregion)
{
	int max = *std::max_element(label_superregion.begin(), label_superregion.end());
	for (int i = 0; i<max; i++)
	{
		double a, b, c = 0;
		int count = 0;
		for (int j = 0; j<extracted_planes.size(); ++j)
		{
			if (label_superregion[j] != i)
				continue;
			Plane plan = extracted_planes[j].plane();
			a += plan.a();
			b += plan.b();
			c += plan.c();
			count++;
		}
		a = a / count;
		b = b / count;
		c = c / count;
		for (int j = 0; j<extracted_planes.size(); ++j)
		{
			if (label_superregion[j] != i)
				continue;
			double d = extracted_planes[j].plane().d();
			Plane p(a, b, c, d);
			extracted_planes[j].set_plane(p);
		}
	}
}

  //-----------------------//
 //---PRIMITIVE-MERGING---//
//------------------------//

/**
* Forms groups of primitives according to a given criterion:
* - NONE: identity function
* - PARA: merge parallel primitives
* - COPL: merge coplanar primitives
* - PARA_ORTH: merge parallel or orthogonal primitives
* @param angle_max angular tolerance for detecting parallel or orthogonal planes
* @param mode primitive merging method, among: NONE (no primitive merging), PARA (will merge parallel primitive, with an angular tolerance angle_max_merge), COPL (will merge coplanar primitive), PARA_ORTH (will merge parallel and orthogonal primitives)
*/
std::vector<int> Cloud_segmentation::merge_primitive(float angle_max, string mode)
{
	time_t begin_timer;
	time_t end_timer;
	time(&begin_timer);

	cout << "Region clustering:  ";
		
	std::vector<int> label_superregion(extracted_planes.size(), -1);
	if (mode == "NONE")
	{
		for (int i = 0; i < extracted_planes.size(); i++)
			label_superregion[i] = i;
		return label_superregion;
	}

	int superclass_index = -1;
	int rank = 0;
	for (std::vector<subPlane>::iterator this_plane = extracted_planes.begin(); this_plane != extracted_planes.end(); ++this_plane, ++rank)
	{
		if (label_superregion[rank] == -1)
		{
			superclass_index++;
			label_superregion[rank] = superclass_index;
			int other_rank = 0;
			for (std::vector<subPlane>::iterator other_plane = extracted_planes.begin(); other_plane != extracted_planes.end(); ++other_plane, other_rank++)
			{
				if (mode == "COPL")
				{
					if (this_plane->is_coplanar_to(*other_plane, angle_max))
						label_superregion[other_rank] = superclass_index;
				}
				else if (mode == "PARA")
				{
					if (this_plane->is_parallel_to(*other_plane, angle_max))
						label_superregion[other_rank] = superclass_index;
				}
				else if (mode == "PARA_ORTH")
				{
					if (this_plane->is_parallel_to(*other_plane, angle_max) || this_plane->is_orthogonal_to(*other_plane, angle_max))
						label_superregion[other_rank] = superclass_index;
				}	
			}
		}
	}
	/* if (mode == "COPL")
		align(label_superregion);
	if (mode == "PARA")
		rotate(label_superregion);
		*/
	return label_superregion;
}

// ---------------- VERBOSE METHODS ---------------- //

/**
* Shows progress of the primitive detection process
* @param ind number of loop
* @param tot total number of loops
*/
void progress_verbose(int ind, int tot)
{
	for (int k = 0; k < 11; k++)
	{
		if (ind == k*(tot / 10))
			cout << "Primitive detection ----------- " << 10 * k << "% completed" << endl;
	}
}

protected:

};

/**
* Shows help to launch program, with all fields and options
*/
void init_verbose()
{
	cout << "--------" << endl;
	cout << "- Aide -" << endl;
	cout << "--------" << endl << endl;
	cout << "L'algorithme nécessite les parametres suivants:" << endl;
	cout << "'tp-ms.exe argv[1] argv[2] argv[3] argv[4]', ou:" << endl;
	cout << "    argv[1]: adresse relative de des donnees a charger" << endl;
	cout << "      Ex: .. / .. / .. / data / p3.off" << endl;
	cout << "    argv[2]: methode d'evaluation des facettes pour le regroupement de primitives" << endl;
	cout << "      Parmis: NORMALS, PRIMFIT, PRIMFIT2" << endl;
	cout << "    argv[3]: critere de regroupement des facettes" << endl;
	cout << "      Parmis:" << endl;
	cout << "        - NONE          aucun regroupement de facettes				detection de primitives" << endl;
	cout << "        - COPL			regroupement de primitives coplanaires		detection de primitives->regroupement des prim.coplanaires" << endl;
	cout << "        - PARA			regroupement de primitives paralleles		detection de primitives->regroupement des prim.paralleles entre elles" << endl;
	cout << "        - PARA_ORTH		regroupement de primitives orth || para		detection de primitives->regroupement des prim.para ou orth entre elles" << endl;
	cout << "    argv[4]: parametres des methodes employees" << endl;
	cout << "      STD pour des parametres standards (bon resultats avec la methode NORMALS)" << endl;
}

#endif 