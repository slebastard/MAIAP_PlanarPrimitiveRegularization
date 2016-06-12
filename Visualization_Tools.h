#ifndef VISUALIZATION_TOOLS_H
#define VISUALIZATION_TOOLS_H

#include "types.h"
#include "cloud_segmentation.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <fstream>
#include <string>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/Polyhedron_IOstream.h>
#include <CGAL/IO/print_wavefront.h>
#include <boost/filesystem.hpp>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <cmath>
#include <math.h>
#include <map>

#define M_PI			3.141592
#define RAD2DEG		180.0/M_PI
#define DEG2RAD		M_PI/180.0
typedef Polyhedron::HalfedgeDS             HalfedgeDS;
typedef Kernel::Point_2 Point_2;



#pragma warning(default:4244 4103 4267 4396)

using std::vector;

#define PARAM_OFSTREAM_TEXTE std::ios::out
#define OFSTREAM_TEXTE(nomvar, nomfic) std::ofstream nomvar (nomfic, PARAM_OFSTREAM_TEXTE) ; nomvar.setf(std::ios::fixed);


template <class HDS>
class Build_sphere : public CGAL::Modifier_base<HDS> {
private:
	double msize;
	Point_d mcenter;
	bool mpureTriangle;
	double mstep;
public:
    Build_sphere(Point_d center,double size, bool pureTriangle, double step) {msize = size;mcenter=center;mpureTriangle = pureTriangle; mstep=step;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		//const double step = 60;//works for 30, 45, 90

		int ratiostep = 360/mstep;
		int nb_point = ratiostep*(ratiostep/2-1)+2;		//+2 because of the top and bottom points
		int nb_facet = ratiostep*(ratiostep/2);			//nb facet mixed triangle and quad
		if (mpureTriangle)
			nb_facet=(nb_facet-2*ratiostep)*2 + 2*ratiostep;

		B.begin_surface( nb_point, nb_facet);			//nb point, nb facet
        typedef typename HDS::Vertex   Vertex;
        typedef typename Vertex::Point Point;
        B.add_vertex( Point( mcenter.x(), mcenter.y(), msize+mcenter.z()));//phi = 90
		double cosphi, sinphi, costheta,sintheta;

		//add point
		double phi;
		int count = 0;
		for (double theta = 0;theta<=360-mstep;theta+=mstep)
		{
			
			sintheta = sin(theta*DEG2RAD);costheta = cos(theta*DEG2RAD);
			for (double tp_phi = mstep;tp_phi<=180-mstep;tp_phi+=mstep)//skip phi = 90 and phi = -90
			{
				phi = 90 - tp_phi;count++;
				cosphi = cos(phi*DEG2RAD);sinphi = sin(phi*DEG2RAD);
				B.add_vertex( Point( msize*cosphi*costheta+mcenter.x(), msize*cosphi*sintheta+mcenter.y(), msize*sinphi+mcenter.z()));
			}
		}


		B.add_vertex( Point( mcenter.x(), mcenter.y(), -msize+mcenter.z()));//phi = -90
		
		//add facet (top)
		double step_index = 180/mstep-1;//= 5
		for (int count = 1;count<=360/mstep; count++)
		{
        B.begin_facet();
			B.add_vertex_to_facet( 0);
			B.add_vertex_to_facet( (count-1)*step_index+1);

			if (count==360/mstep)
				B.add_vertex_to_facet(1);
			else
				B.add_vertex_to_facet(count*step_index+1);
        B.end_facet();
		}


		if (mpureTriangle)
		{
			for (int start = 1;start <= step_index-1;start++)
			{
				for (int count = 1;count<=360/mstep; count++)
				{
				B.begin_facet();
					B.add_vertex_to_facet( start+(count-1)*step_index);
					B.add_vertex_to_facet( start+(count-1)*step_index+1);
					if (count==360/mstep)
						B.add_vertex_to_facet( start+1 );
					else
						B.add_vertex_to_facet( start+count*step_index+1 );
				B.end_facet();

				B.begin_facet();
					B.add_vertex_to_facet( start+(count-1)*step_index);
					if (count==360/mstep)
					{
						B.add_vertex_to_facet( start+1 );
						B.add_vertex_to_facet( start);
					}
					else
					{
						B.add_vertex_to_facet( start+count*step_index+1 );
						B.add_vertex_to_facet( start+count*step_index);
					}
				B.end_facet();
				}
			}

		}else{

			for (int start = 1;start <= step_index-1;start++)
			{
				for (int count = 1;count<=360/mstep; count++)
				{
				B.begin_facet();
					B.add_vertex_to_facet( start+(count-1)*step_index);
					B.add_vertex_to_facet( start+(count-1)*step_index+1);
					if (count==360/mstep)
					{
						B.add_vertex_to_facet( start+1 );
						B.add_vertex_to_facet( start);
					}
					else
					{
						B.add_vertex_to_facet( start+count*step_index+1 );
						B.add_vertex_to_facet( start+count*step_index);
					}
				B.end_facet();
				}
			}
		}

			
		//add facet (bottom)
		for (int count = 1;count<=360/mstep; count++)
		{
        B.begin_facet();
			B.add_vertex_to_facet( count*step_index);
			B.add_vertex_to_facet( nb_point-1);

			if (count==360/mstep)
				B.add_vertex_to_facet(step_index);
			else
				B.add_vertex_to_facet((count+1)*step_index);

        B.end_facet();
		}

	
		

        B.end_surface();
    }
};



template <class HDS>
class Build_cylinder : public CGAL::Modifier_base<HDS> {
private:
	double msize;
	Point_d mpt1;
	Point_d mpt2;
	bool mpureTriangle;
	double mstep; //works for  30, 45,60, 90 (in degree)
public:
    Build_cylinder(Point_d pt1,Point_d pt2, double size, bool pureTriangle, double step) {msize = size; mpt1=pt1;  mpt2=pt2; mpureTriangle = pureTriangle; mstep=step;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		//const double step = step_deg;//works for 30, 45, 90

		int ratiostep = 360/mstep;
		int nb_point = 2*ratiostep;		//+2 because of the top and bottom points
		int nb_facet = 2*ratiostep;			//nb facet mixed triangle and quad
		//if (mpureTriangle) nb_facet=(nb_facet-2*ratiostep)*2 + 2*ratiostep;

		B.begin_surface( nb_point, nb_facet);			//nb point, nb facet
       // typedef typename HDS::Vertex   Vertex;

		Line lin(mpt1,mpt2); 
		Plane_3 plane1(mpt1,lin.direction()); 
		Plane_3 plane2(mpt2,lin.direction()); 

		Point_2 ptc=plane1.to_2d(mpt1); 
		std::vector<Point_d> list_pt1;
		std::vector<Point_d> list_pt2;

		for(int i=0;i<ratiostep;i++){
			Point_2 temp_2(cos(i*M_PI*mstep/180),sin(i*M_PI*mstep/180)); 
			Vector_3 B1=plane1.base1(); FT norme1=sqrt(pow(B1.x(),2)+pow(B1.y(),2)+pow(B1.z(),2)); Vector_3 b1=(1/norme1)*B1;		
			Vector_3 B2=plane1.base2(); FT norme2=sqrt(pow(B2.x(),2)+pow(B2.y(),2)+pow(B2.z(),2)); Vector_3 b2=(1/norme2)*B2;	
			Point_d temp_3(mpt1.x()+msize*(b1.x()*temp_2.x()+b2.x()*temp_2.y()),mpt1.y()+msize*(b1.y()*temp_2.x()+b2.y()*temp_2.y()),mpt1.z()+msize*(b1.z()*temp_2.x()+b2.z()*temp_2.y())); 

			list_pt1.push_back(temp_3);
			list_pt2.push_back(plane2.projection(temp_3));

		}


		//add point
		for(int i=0;i<ratiostep;i++) B.add_vertex(list_pt1[i]); //std::cout<<list_pt1[i]<<std::endl;}
		for(int i=0;i<ratiostep;i++) B.add_vertex(list_pt2[i]); //std::cout<<list_pt2[i]<<std::endl;}
		
		//add facet 
		for(int i=0;i<ratiostep-1;i++){
			 B.begin_facet();
			 B.add_vertex_to_facet(i);
			 B.add_vertex_to_facet(i+1);
			 B.add_vertex_to_facet(i+ratiostep);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(i+1);
			 B.add_vertex_to_facet(i+ratiostep+1);
			 B.add_vertex_to_facet(i+ratiostep);
			 B.end_facet();
		}
			 
			 B.begin_facet();
			 B.add_vertex_to_facet(ratiostep-1);
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(2*ratiostep-1);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(ratiostep);
			 B.add_vertex_to_facet(2*ratiostep-1);
			 B.end_facet();


        B.end_surface();
    }
};





template <class HDS>
class Build_square : public CGAL::Modifier_base<HDS> {
private:
	std::vector < Point_d > mvpt;
	bool mpureTriangle;
public:
	Build_square(std::vector < Point_d > vpt, bool pureTriangle) { mvpt=vpt; mpureTriangle = pureTriangle;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

	

		B.begin_surface(4,2);			//nb point, nb facet
  

		//add point
		for(int i=0;i<4;i++) B.add_vertex(mvpt[i]); 
		
		
		//add facet 
		
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(2);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(0);
			 B.end_facet();
		
        B.end_surface();
    }
};





template <class HDS>
class Build_cube : public CGAL::Modifier_base<HDS> {
private:
	std::vector < Point_d > mvpt;
	bool mpureTriangle;
public:
	Build_cube(std::vector < Point_d > vpt, bool pureTriangle) { mvpt=vpt; mpureTriangle = pureTriangle;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

	

		B.begin_surface(8,12);			//nb point, nb facet
  

		//add point
		for(int i=0;i<8;i++) B.add_vertex(mvpt[i]); 
		
		
		//add facet 
		
			/* B.begin_facet();
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(0);
			 B.end_facet();*/

			 B.begin_facet();
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(1);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(3);
			 B.end_facet();

/*
			 B.begin_facet();
			 B.add_vertex_to_facet(4);
			 B.add_vertex_to_facet(5);
			 B.add_vertex_to_facet(6);
			 B.add_vertex_to_facet(7);
			 B.end_facet();*/

			 B.begin_facet();
			 B.add_vertex_to_facet(4);
			 B.add_vertex_to_facet(5);
			 B.add_vertex_to_facet(6);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(6);
			 B.add_vertex_to_facet(7);
			 B.add_vertex_to_facet(4);
			 B.end_facet();

			 /*
			 B.begin_facet();
			 B.add_vertex_to_facet(4);
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(5);
			 B.end_facet();*/

			 B.begin_facet();
			 B.add_vertex_to_facet(4);
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(1);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(5);
			 B.add_vertex_to_facet(4);
			 B.end_facet();

				/*
			 B.begin_facet();
			 B.add_vertex_to_facet(5);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(6);
			 B.end_facet();*/

			 B.begin_facet();
			 B.add_vertex_to_facet(5);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(2);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(6);
			 B.add_vertex_to_facet(5);
			 B.end_facet();

			 /*
			 B.begin_facet();
			 B.add_vertex_to_facet(6);
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(7);
			 B.end_facet();*/

			 B.begin_facet();
			 B.add_vertex_to_facet(6);
			 B.add_vertex_to_facet(2);
			 B.add_vertex_to_facet(3);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(7);
			 B.add_vertex_to_facet(6);
			 B.end_facet();

			 /*
			 B.begin_facet();
			 B.add_vertex_to_facet(7);
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(4);
			 B.end_facet();*/

			  B.begin_facet();
			 B.add_vertex_to_facet(7);
			 B.add_vertex_to_facet(3);
			 B.add_vertex_to_facet(0);
			 B.end_facet();

			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(4);
			 B.add_vertex_to_facet(7);
			 B.end_facet();
			

		
        B.end_surface();
    }
};



template <class HDS>
class Build_circle : public CGAL::Modifier_base<HDS> {
private:
	double mradius;
	Point_d mpt;
	Vector_3 mnormal;
	bool mpureTriangle;
public:
    Build_circle(Point_d pt, Vector_3 normal, double radius, bool pureTriangle) {mradius = radius; mpt=pt;  mnormal=normal; mpureTriangle = pureTriangle;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		const double step = 30; //degree

		int ratiostep = 360/step;
		int nb_point = ratiostep+1;	
		int nb_facet = ratiostep;	

		B.begin_surface( nb_point, nb_facet);			//nb point, nb facet
  
		Plane_3 plane(mpt,mnormal); 
		Point_2 ptc=plane.to_2d(mpt); 
		std::vector<Point_d> list_pt1;

		for(int i=0;i<ratiostep;i++){
			Point_2 temp_2(cos(i*M_PI*step/180),sin(i*M_PI*step/180)); 
			Vector_3 B1=plane.base1(); FT norme1=sqrt(pow(B1.x(),2)+pow(B1.y(),2)+pow(B1.z(),2)); Vector_3 b1=(1/norme1)*B1;		
			Vector_3 B2=plane.base2(); FT norme2=sqrt(pow(B2.x(),2)+pow(B2.y(),2)+pow(B2.z(),2)); Vector_3 b2=(1/norme2)*B2;	
			Point_d temp_3(mpt.x()+mradius*(b1.x()*temp_2.x()+b2.x()*temp_2.y()),mpt.y()+mradius*(b1.y()*temp_2.x()+b2.y()*temp_2.y()),mpt.z()+mradius*(b1.z()*temp_2.x()+b2.z()*temp_2.y())); 

			list_pt1.push_back(temp_3);
		}


		//add point
		B.add_vertex(mpt);
		for(int i=0;i<ratiostep;i++) B.add_vertex(list_pt1[i]); 
	
		
		//add facet 
		for(int i=1;i<ratiostep;i++){
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(i);
			 B.add_vertex_to_facet(i+1);
			 B.end_facet();
		}
			 
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(ratiostep);
			 B.add_vertex_to_facet(1);
			 B.end_facet();

        B.end_surface();
    }
};





template <class HDS>
class Build_ellipse : public CGAL::Modifier_base<HDS> {
private:
	double ma;
	double mb;
	Point_d mpt;
	Vector_3 mnormal;
	int mstep;
	bool mpureTriangle;
public:
    Build_ellipse(Point_d pt, Vector_3 normal, double a, double b, int step, bool pureTriangle) {mstep=step; ma = a; mb = b; mpt=pt;  mnormal=normal; mpureTriangle = pureTriangle;}
    void operator()( HDS& hds) {
        
		// Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);
		

		int ratiostep = 360/mstep;
		int nb_point = ratiostep+1;	
		int nb_facet = ratiostep;	

		B.begin_surface( nb_point, nb_facet);			//nb point, nb facet
  
		Plane_3 plane(mpt,mnormal); 
		Point_2 ptc=plane.to_2d(mpt); 
		std::vector<Point_d> list_pt1;

		for(int i=0;i<ratiostep;i++){

			Point_2 temp_2;
			double x=cos(i*M_PI*mstep/180);
			double b_norm=mb/ma;
			if((double)i*mstep/180<=1.) temp_2= Point_2(x,b_norm*sqrt(1-x*x));
			else temp_2= Point_2(x,-b_norm*sqrt(1-x*x));

			Vector_3 B1=plane.base1(); FT norme1=sqrt(pow(B1.x(),2)+pow(B1.y(),2)+pow(B1.z(),2)); Vector_3 b1=(1/norme1)*B1;		
			Vector_3 B2=plane.base2(); FT norme2=sqrt(pow(B2.x(),2)+pow(B2.y(),2)+pow(B2.z(),2)); Vector_3 b2=(1/norme2)*B2;	
			Point_d temp_3(mpt.x()+ma*(b1.x()*temp_2.x()+b2.x()*temp_2.y()),mpt.y()+ma*(b1.y()*temp_2.x()+b2.y()*temp_2.y()),mpt.z()+ma*(b1.z()*temp_2.x()+b2.z()*temp_2.y())); 
			list_pt1.push_back(temp_3);
		}


		//add point
		B.add_vertex(mpt);
		for(int i=0;i<ratiostep;i++) B.add_vertex(list_pt1[i]); 
	
		
		//add facet 
		for(int i=1;i<ratiostep;i++){
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(i);
			 B.add_vertex_to_facet(i+1);
			 B.end_facet();
		}
			 
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(ratiostep);
			 B.add_vertex_to_facet(1);
			 B.end_facet();

        B.end_surface();
    }
};





template <class HDS>
class Build_cone : public CGAL::Modifier_base<HDS> {
private:
	double mradius;
	double mheight;
	Point_d mpt;
	Vector_3 mnormal;
	bool mpureTriangle;
public:
    Build_cone(Point_d pt, Vector_3 normal, double radius, double height, bool pureTriangle) {mradius = radius; mheight=height; mpt=pt;  mnormal=normal; mpureTriangle = pureTriangle;}
    void operator()( HDS& hds) {
        // Postcondition: `hds' is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		const double step = 5; //degree

		int ratiostep = 360/step;
		int nb_point = ratiostep+2;	
		int nb_facet = 2*ratiostep;	

		B.begin_surface( nb_point, nb_facet);			//nb point, nb facet
  
		double norm=sqrt(pow(mnormal.x(),2)+pow(mnormal.y(),2)+pow(mnormal.z(),2)); if(norm<1e-7) norm=1e-7;
		Vector_3 normal_normalise(mnormal.x()/norm,mnormal.y()/norm,mnormal.z()/norm);
		Point_d pth(mpt.x()+mheight*normal_normalise.x(),mpt.y()+mheight*normal_normalise.y(),mpt.z()+mheight*normal_normalise.z());

		Plane_3 plane(mpt,mnormal); 
		Point_2 ptc=plane.to_2d(mpt); 
		std::vector<Point_d> list_pt1;

		for(int i=0;i<ratiostep;i++){
			Point_2 temp_2(cos(i*M_PI*step/180),sin(i*M_PI*step/180)); 
			Vector_3 B1=plane.base1(); FT norme1=sqrt(pow(B1.x(),2)+pow(B1.y(),2)+pow(B1.z(),2)); Vector_3 b1=(1/norme1)*B1;		
			Vector_3 B2=plane.base2(); FT norme2=sqrt(pow(B2.x(),2)+pow(B2.y(),2)+pow(B2.z(),2)); Vector_3 b2=(1/norme2)*B2;	
			Point_d temp_3(mpt.x()+mradius*(b1.x()*temp_2.x()+b2.x()*temp_2.y()),mpt.y()+mradius*(b1.y()*temp_2.x()+b2.y()*temp_2.y()),mpt.z()+mradius*(b1.z()*temp_2.x()+b2.z()*temp_2.y())); 

			list_pt1.push_back(temp_3);
		}


		//add point
		B.add_vertex(pth);
		B.add_vertex(mpt);
		for(int i=0;i<ratiostep;i++) B.add_vertex(list_pt1[i]); 
	
		
		//add facet 
		for(int i=2;i<ratiostep+1;i++){
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(i);
			 B.add_vertex_to_facet(i+1);
			 B.end_facet();
		}
			 
			 B.begin_facet();
			 B.add_vertex_to_facet(0);
			 B.add_vertex_to_facet(ratiostep+1);
			 B.add_vertex_to_facet(2);
			 B.end_facet();


		for(int i=2;i<ratiostep+1;i++){
			 B.begin_facet();
			 B.add_vertex_to_facet(i);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(i+1);
			 B.end_facet();
		}
			 
			 B.begin_facet();
			 B.add_vertex_to_facet(ratiostep+1);
			 B.add_vertex_to_facet(1);
			 B.add_vertex_to_facet(2);
			 B.end_facet();

        B.end_surface();
    }
};






Polyhedron createDisk(Point_d p, Vector_3 normal, double radius)
{
	Polyhedron P;
	Build_circle<HalfedgeDS> poly1(p,normal,radius,true);
	P.delegate(poly1);

	return P;
}
Polyhedron createEllipse(Point_d p, Vector_3 normal, double a, double b,int step)
{
	Polyhedron P;
	Build_ellipse<HalfedgeDS> poly1(p,normal,a,b,step,true);
	P.delegate(poly1);

	return P;
}
Polyhedron createCone(Point_d p, Vector_3 normal, double radius, double height)
{
	Polyhedron P;
	Build_cone<HalfedgeDS> poly1(p,normal,radius,height,true);
	P.delegate(poly1);

	return P;
}



Polyhedron createSquare(std::vector < Point_d > vp)
{
	Polyhedron P;
	Build_square<HalfedgeDS> poly1(vp,true);
	P.delegate(poly1);

	return P;
}




















Polyhedron createCube(std::vector < Point_d > vp)
{
	Polyhedron P;
	Build_cube<HalfedgeDS> poly1(vp,true);
	P.delegate(poly1);

	return P;
}


Polyhedron createArrow(Point_d p1, Point_d p2, double size)
{
	Polyhedron P;

	double pas=3.5;

	Vector_3 normal_temp(p1.x()-p2.x(),p1.y()-p2.y(),p1.z()-p2.z()); double norm=normal_temp.squared_length(); if(norm<1e-7) norm = 1e-7;
	Vector_3 normal(normal_temp.x()/norm, normal_temp.y()/norm, normal_temp.z()/norm);
	Point_d pt1(p1.x()-normal.x()*pas*size,p1.y()-normal.y()*pas*size, p1.z()-normal.z()*pas*size);
	Point_d pt2(p2.x()+normal.x()*pas*size,p2.y()+normal.y()*pas*size, p2.z()+normal.z()*pas*size);

/*	Build_cylinder<HalfedgeDS> poly1(pt1,pt2,size,true);
	P.delegate(poly1);*/

	if(normal.x()<0){
	/*	Build_cone<HalfedgeDS> poly2(pt1,-normal,pas/2*size,pas*size,true);
		P.delegate(poly2);*/

		Build_cone<HalfedgeDS> poly3(pt2,normal,pas/2*size,pas*size,true);
		P.delegate(poly3);
	}
	else{
/*		Build_cone<HalfedgeDS> poly2(pt1,normal,pas/2*size,pas*size,true);
		P.delegate(poly2);*/

	Build_cone<HalfedgeDS> poly3(pt2,-normal,pas/2*size,pas*size,true);
		P.delegate(poly3);
	}

	

	return P;
}





















Polyhedron createDashedCylinder(Point_d p1, Point_d p2, double size)
{
	double pas=0.5/4;//0.03;
	double step=45;
	//double length=sqrt(pow(p1.x()-p2.x(),2)+pow(p1.y()-p2.y(),2)+pow(p1.z()-p2.z(),2));
	Vector_3 normal(p2,p1); //double norm=sqrt(pow(normal
	Point_d p1a(p1.x()+pas*normal.x(),p1.y()+pas*normal.y(),p1.z()+pas*normal.z());
	Point_d p2a(p1.x()+2*pas*normal.x(),p1.y()+2*pas*normal.y(),p1.z()+2*pas*normal.z());
	Point_d p3a(p1.x()+3*pas*normal.x(),p1.y()+3*pas*normal.y(),p1.z()+3*pas*normal.z());
	Point_d p4a(p1.x()+4*pas*normal.x(),p1.y()+4*pas*normal.y(),p1.z()+4*pas*normal.z());
	
	Point_d p1b(p2.x()-pas*normal.x(),p2.y()-pas*normal.y(),p2.z()-pas*normal.z());
	Point_d p2b(p2.x()-2*pas*normal.x(),p2.y()-2*pas*normal.y(),p2.z()-2*pas*normal.z());
	Point_d p3b(p2.x()-3*pas*normal.x(),p2.y()-3*pas*normal.y(),p2.z()-3*pas*normal.z());
	Point_d p4b(p2.x()-4*pas*normal.x(),p2.y()-4*pas*normal.y(),p2.z()-4*pas*normal.z());

	Polyhedron P;
	Build_cylinder<HalfedgeDS> poly1(p1,p2,size,true,step);
	P.delegate(poly1);
	
	Build_cylinder<HalfedgeDS> poly2(p1a,p2a,size,true,step);
	P.delegate(poly2);
	Build_cylinder<HalfedgeDS> poly3(p3a,p4a,size,true,step);
	P.delegate(poly3);
	Build_cylinder<HalfedgeDS> poly4(p1b,p2b,size,true,step);
	P.delegate(poly4);
	Build_cylinder<HalfedgeDS> poly5(p3b,p4b,size,true,step);
	P.delegate(poly5);

	Build_sphere<HalfedgeDS> poly1a(p1a,size,true,step);
	P.delegate(poly1a);
	Build_sphere<HalfedgeDS> poly2a(p2a,size,true,step);
	P.delegate(poly2a);
	Build_sphere<HalfedgeDS> poly3a(p3a,size,true,step);
	P.delegate(poly3a);
	Build_sphere<HalfedgeDS> poly4a(p4a,size,true,step);
	P.delegate(poly4a);

	Build_sphere<HalfedgeDS> poly1b(p1b,size,true,step);
	P.delegate(poly1b);
	Build_sphere<HalfedgeDS> poly2b(p2b,size,true,step);
	P.delegate(poly2b);
	Build_sphere<HalfedgeDS> poly3b(p3b,size,true,step);
	P.delegate(poly3b);
	Build_sphere<HalfedgeDS> poly4b(p4b,size,true,step);
	P.delegate(poly4b);


	return P;
}


Polyhedron createDashedCylinder2(Point_d p1, Point_d p2, double size)
{
	double pas=0.03;//0.03;
	double step=45;
	//double length=sqrt(pow(p1.x()-p2.x(),2)+pow(p1.y()-p2.y(),2)+pow(p1.z()-p2.z(),2));
	Vector_3 normal(p2,p1); //double norm=sqrt(pow(normal
	Point_d p1a(p1.x()-pas*normal.x(),p1.y()-pas*normal.y(),p1.z()-pas*normal.z());
	Point_d p2a(p1.x()-2*pas*normal.x(),p1.y()-2*pas*normal.y(),p1.z()-2*pas*normal.z());
	Point_d p3a(p1.x()-3*pas*normal.x(),p1.y()-3*pas*normal.y(),p1.z()-3*pas*normal.z());
	Point_d p4a(p1.x()-4*pas*normal.x(),p1.y()-4*pas*normal.y(),p1.z()-4*pas*normal.z());
	
	Point_d p1b(p2.x()+pas*normal.x(),p2.y()+pas*normal.y(),p2.z()+pas*normal.z());
	Point_d p2b(p2.x()+2*pas*normal.x(),p2.y()+2*pas*normal.y(),p2.z()+2*pas*normal.z());
	Point_d p3b(p2.x()+3*pas*normal.x(),p2.y()+3*pas*normal.y(),p2.z()+3*pas*normal.z());
	Point_d p4b(p2.x()+4*pas*normal.x(),p2.y()+4*pas*normal.y(),p2.z()+4*pas*normal.z());

	Polyhedron P;
	Build_cylinder<HalfedgeDS> poly1(p4a,p4b,size,true,step);
	P.delegate(poly1);
	
	Build_cylinder<HalfedgeDS> poly2(p2a,p3a,size,true,step);
	P.delegate(poly2);
	Build_cylinder<HalfedgeDS> poly3(p1,p1a,size,true,step);
	P.delegate(poly3);
	Build_cylinder<HalfedgeDS> poly4(p2b,p3b,size,true,step);
	P.delegate(poly4);
	Build_cylinder<HalfedgeDS> poly5(p2,p1b,size,true,step);
	P.delegate(poly5);

	Build_sphere<HalfedgeDS> poly1a(p1a,size,true,step);
	P.delegate(poly1a);
	Build_sphere<HalfedgeDS> poly2a(p2a,size,true,step);
	P.delegate(poly2a);
	Build_sphere<HalfedgeDS> poly3a(p3a,size,true,step);
	P.delegate(poly3a);
	Build_sphere<HalfedgeDS> poly4a(p4a,size,true,step);
	P.delegate(poly4a);

	Build_sphere<HalfedgeDS> poly1b(p1b,size,true,step);
	P.delegate(poly1b);
	Build_sphere<HalfedgeDS> poly2b(p2b,size,true,step);
	P.delegate(poly2b);
	Build_sphere<HalfedgeDS> poly3b(p3b,size,true,step);
	P.delegate(poly3b);
	Build_sphere<HalfedgeDS> poly4b(p4b,size,true,step);
	P.delegate(poly4b);


	return P;
}

Polyhedron createSphere(Point_d p, double size)
{
	Polyhedron P;
	double step=60;
	Build_sphere<HalfedgeDS> poly1(p,size,true,step);
	P.delegate(poly1);

	return P;
}

Polyhedron createSphere_fine(Point_d p, double size)
{
	Polyhedron P;
	double step=30;
	Build_sphere<HalfedgeDS> poly1(p,size,true,step);
	P.delegate(poly1);

	return P;
}

Polyhedron createCylinder(Point_d p1, Point_d p2, double size)
{
	Polyhedron P;
	double step=90;
	Build_cylinder<HalfedgeDS> poly1(p1,p2,size,true,step);
	P.delegate(poly1);

	return P;
}

Polyhedron createCylinder_fine(Point_d p1, Point_d p2, double size)
{
	Polyhedron P;
	double step=30;
	Build_cylinder<HalfedgeDS> poly1(p1,p2,size,true,step);
	P.delegate(poly1);

	return P;
}





























bool save_listpolyhedron2ply(std::vector<Polyhedron> lP, char *nom, std::vector<CGAL::Color> lC)
{
	std::cout <<"Saving polyhedron to "<<nom<<std::endl;
	OFSTREAM_TEXTE(fic,nom);
	
	int nb_vertices = 0;
	int nb_facets = 0;

	std::vector<Polyhedron>::iterator it;
	for(it = lP.begin(); it != lP.end(); it++)
	{				
		nb_vertices+=(*it).size_of_vertices ();
		nb_facets+=(*it).size_of_facets ();
	}

	//header
	fic<<"ply"<<std::endl;
	fic<<"format ascii 1.0"<<std::endl;
	fic<<"element vertex "<<nb_vertices<<std::endl;
	fic<<"property float x"<<std::endl;
	fic<<"property float y"<<std::endl;
	fic<<"property float z"<<std::endl;
	fic<<"element face "<<nb_facets<<std::endl;
	fic<<"property list uchar int vertex_index"<<std::endl;
	fic<<"property uchar red"<<std::endl;
	fic<<"property uchar green"<<std::endl;
	fic<<"property uchar blue"<<std::endl;
	fic<<"end_header"<<std::endl;

	//vertex list
	std::map<Point_d,int> map_point_index;
	int count = 0;
	for(it = lP.begin(); it != lP.end(); it++)
	{			
		for( Polyhedron::Vertex_const_iterator vi = (*it).vertices_begin(); vi != (*it).vertices_end(); ++vi) 
		{
			map_point_index[vi->point()] = count++;
			fic<<vi->point().x()<<" "<<vi->point().y()<<" "<<vi->point().z()<<std::endl;
		}
	}

	int i=0;
	for(it = lP.begin(); it != lP.end(); it++, i++)
	{	
		//facet list
		Polyhedron::Halfedge_around_facet_const_circulator hc, hc_end;
		for(  Polyhedron::Facet_const_iterator fi = (*it).facets_begin(); fi != (*it).facets_end(); ++fi)
		{
			hc = fi->facet_begin();
			hc_end = hc;
			std::size_t n = circulator_size( hc);
			CGAL_assertion( n >= 3);
			fic<<n<<" ";
			do {
				fic<<map_point_index[hc->vertex()->point()]<<" ";
				++hc;
			} while( hc != hc_end);
			fic<<(int)lC[i].r()<<" "<<(int)lC[i].g()<<" "<<(int)lC[i].b()<<std::endl;
		}
	}


		
fic<<std::endl;	

return true;
}









bool colorplanset2ply( char * filename, std::vector < std::vector < Point_d > > & pt, std::vector< CGAL::Color > & colors)
	{

	std::cout <<"Saving planes to "<<filename<<std::endl;
	OFSTREAM_TEXTE(fic,filename);
	
	int nb_pts=0;
	for(int i=0;i<pt.size();i++){nb_pts=nb_pts+pt[i].size();}
		

	//header
		fic<<"ply"<<std::endl;
		fic<<"format ascii 1.0"<<std::endl;
		fic<<"comment author: F. Lafarge"<<std::endl;
		fic<<"element vertex "<<nb_pts<<std::endl;
		fic<<"property float x"<<std::endl;
		fic<<"property float y"<<std::endl;
		fic<<"property float z"<<std::endl;
		fic<<"property uchar red"<<std::endl;
		fic<<"property uchar green"<<std::endl;
		fic<<"property uchar blue"<<std::endl;
		fic<<"end_header"<<std::endl;

		//vertex list

		for(int i=0;i<pt.size();i++){
			for(int j=0;j<pt[i].size();j++){
				fic<<pt[i][j].x()<<" "<<pt[i][j].y()<<" "<<pt[i][j].z()<<" "<<(int)colors[i].r()<<" "<<(int)colors[i].g()<<" "<<(int)colors[i].b()<<std::endl;
			}
		}

fic<<std::endl;	

return true;
	}










template <class Cloud_segmentation>
bool save_envelops(Cloud_segmentation C)
	{

		std::vector < std::vector < Point_d > > point3dres_graham;
		std::vector < std::vector < Point_d > > point3dres_alpha_shape;
		
		std::vector < CGAL::Color > color_col_graham;
		std::vector < CGAL::Color > color_col_alpha_shape;

	/*	std::vector<Polyhedron> lp;
		std::vector<CGAL::Color> lc;
		std::vector<Polyhedron> lp_centroid;
		std::vector<CGAL::Color> lc_centroid;*/

		int val_lab=0;
		int counting1=0;

		std::vector<CGAL::Color> vector_of_colors;
		for(int i=0; i<(int)C.plane_point_index.size();i++){
			int red=(int)floor((double)126*rand()/RAND_MAX)+130;
			int green=(int)floor((double)126*rand()/ RAND_MAX)+130;
			int blue=(int)floor((double)126*rand()/ RAND_MAX)+130;
			CGAL::Color col(std::max(0,std::min(255,red)),std::max(0,std::min(255,green)),std::max(0,std::min(255,blue)),120);
			vector_of_colors.push_back(col);
		}
		
		std::vector < std::vector < Point_d > > listpts;

		for(int i=0; i<(int)C.plane_point_index.size();i++){
			
			std::vector < Point_d > listp;
			for(int k=0; k<(int)C.plane_point_index[i].size(); k++){
				int yy=C.plane_point_index[i][k];
				Point_d pt=C.HPS[yy].position;
				listp.push_back(pt);
			}
			listpts.push_back(listp);

			val_lab++;
			Plane_3 plane;
			double erro=linear_least_squares_fitting_3(listp.begin(),listp.end(),plane, CGAL::Dimension_tag<0>());
			C.extracted_planes.push_back(plane);

			CGAL::Color col=vector_of_colors[i];
			
			
			Vector_3 vortho=plane.orthogonal_vector();
			Vector_3 b1=plane.base1();
			Vector_3 b2=plane.base2();
			FT norme1=sqrt(pow(b1.x(),2)+pow(b1.y(),2)+pow(b1.z(),2)); if(norme1<1e-7){norme1=1e-7;}
			FT norme2=sqrt(pow(b2.x(),2)+pow(b2.y(),2)+pow(b2.z(),2)); if(norme2<1e-7){norme2=1e-7;}

			Vector_3 vb1=(1/norme1)*b1;
			Vector_3 vb2=(1/norme2)*b2;

			std::vector< Point_d > poinn;
			std::vector< Point_2d > poin2;

			for(int k=0; k<(int)C.plane_point_index[i].size();k++){
				int ind=C.plane_point_index[i][k];
				Point_d pt=C.HPS[ind].position;
				Point_d ptp=plane.projection(pt);
				FT X1=vb1.x()*ptp.x()+vb1.y()*ptp.y()+vb1.z()*ptp.z();
				FT X2=vb2.x()*ptp.x()+vb2.y()*ptp.y()+vb2.z()*ptp.z();
				Point_2d ptp2(X1,X2);
				poinn.push_back(ptp);
				poin2.push_back(ptp2);
			}

			std::vector < Point_2d > poin2res_graham;
			std::vector < Point_2d > poin2res_alphashape;
			CGAL::ch_graham_andrew(poin2.begin(),poin2.end(),std::back_inserter(poin2res_graham));
			Alpha_Shape AlphaShape(poin2.begin(),poin2.end(),FT(0.005*C.BBox_diagonal),Alpha_Shape::REGULARIZED);   
			
			std::vector < std::pair < Point_2d, Point_2d> > pairs_edges;
			std::vector < bool > active_pairs;
			for (Alpha_Shape::Alpha_shape_edges_iterator it = AlphaShape.alpha_shape_edges_begin(); it != AlphaShape.alpha_shape_edges_end(); ++it){
				Alpha_Shape::Edge edg= *it;
				Point_2d pe1=edg.first->vertex(edg.first->ccw(edg.second))->point();
				Point_2d pe2=edg.first->vertex(edg.first->cw(edg.second))->point();
			
				std::pair < Point_2d, Point_2d> pair_ed;
				pair_ed.first=pe1;
				pair_ed.second=pe2;
				pairs_edges.push_back(pair_ed);
				active_pairs.push_back(true);
			}

			std::vector < Point_2d > ranked_pts;
			if(pairs_edges.size()>0){
				ranked_pts.push_back(pairs_edges[0].first); 
				ranked_pts.push_back(pairs_edges[0].second); 
				active_pairs[0]=false;
				Point_2d pt_to_find=pairs_edges[0].second;
				do{
					for(int iter=1;iter<pairs_edges.size();iter++){
						if(active_pairs[iter]){
							if(pt_to_find == pairs_edges[iter].first) { ranked_pts.push_back( pairs_edges[iter].second);  pt_to_find=pairs_edges[iter].second; active_pairs[iter]=false; }
							else if(pt_to_find == pairs_edges[iter].second) { ranked_pts.push_back( pairs_edges[iter].first); pt_to_find=pairs_edges[iter].first; active_pairs[iter]=false; }
						}
					}
				}while(pt_to_find!=pairs_edges[0].first);
			}

			Polygon_2 poly2(ranked_pts.begin(),ranked_pts.end());
			CDT2 cdt;

			for(Polygon_2::Vertex_iterator it2 = poly2.vertices_begin () ; it2 != poly2.vertices_end ();++it2) cdt.insert(*it2);

			CGAL::make_conforming_Delaunay_2(cdt);


			std::vector < Polygon_2 >  partition_polys;
			for ( CDT2::Finite_faces_iterator  vit = cdt.finite_faces_begin(); vit != cdt.finite_faces_end(); ++vit){        
                    vector<Point_2d> p(3);
                    p[0] = vit->vertex (0)->point();
                    p[1] = vit->vertex (1)->point();
                    p[2] = vit->vertex (2)->point();
                    Point_2d bary( (p[0].x()+ p[1].x() + p[2].x())/3.,(p[0].y()+ p[1].y() + p[2].y())/3.);
    
					if (CGAL::bounded_side_2(poly2.vertices_begin (), poly2.vertices_end (),bary, Kernel()) != CGAL::ON_UNBOUNDED_SIDE)
                    {
                        Polygon_2 po(p.begin(),p.end());
                        partition_polys.push_back(po);
						vector<Point_d> pd(3);
						FT X1=p[0].x()*vb1.x()+p[0].y()*vb2.x()-plane.d()*vortho.x();
						FT Y1=p[0].x()*vb1.y()+p[0].y()*vb2.y()-plane.d()*vortho.y();
						FT Z1=p[0].x()*vb1.z()+p[0].y()*vb2.z()-plane.d()*vortho.z();
						pd[0] = Point_d(X1,Y1,Z1); 
						FT X2=p[1].x()*vb1.x()+p[1].y()*vb2.x()-plane.d()*vortho.x();
						FT Y2=p[1].x()*vb1.y()+p[1].y()*vb2.y()-plane.d()*vortho.y();
						FT Z2=p[1].x()*vb1.z()+p[1].y()*vb2.z()-plane.d()*vortho.z();
						pd[1] = Point_d(X2,Y2,Z2); 
						FT X3=p[2].x()*vb1.x()+p[2].y()*vb2.x()-plane.d()*vortho.x();
						FT Y3=p[2].x()*vb1.y()+p[2].y()*vb2.y()-plane.d()*vortho.y();
						FT Z3=p[2].x()*vb1.z()+p[2].y()*vb2.z()-plane.d()*vortho.z();
						pd[2] = Point_d(X3,Y3,Z3); 
						point3dres_alpha_shape.push_back(pd);
						color_col_alpha_shape.push_back(col);
                    }
			}

		/*	if(ranked_pts.size()>1){
				for(int k=0; k<(int)ranked_pts.size()-1;k++){
					FT X1a=ranked_pts[k].x()*vb1.x()+ranked_pts[k].y()*vb2.x()-plane.d()*vortho.x();
					FT X2a=ranked_pts[k].x()*vb1.y()+ranked_pts[k].y()*vb2.y()-plane.d()*vortho.y();
					FT X3a=ranked_pts[k].x()*vb1.z()+ranked_pts[k].y()*vb2.z()-plane.d()*vortho.z();
					FT X1b=ranked_pts[k+1].x()*vb1.x()+ranked_pts[k+1].y()*vb2.x()-plane.d()*vortho.x();
					FT X2b=ranked_pts[k+1].x()*vb1.y()+ranked_pts[k+1].y()*vb2.y()-plane.d()*vortho.y();
					FT X3b=ranked_pts[k+1].x()*vb1.z()+ranked_pts[k+1].y()*vb2.z()-plane.d()*vortho.z();
					Point_d tempoA(X1a,X2a,X3a),tempoB(X1b,X2b,X3b);
					lp.push_back(createCylinder(tempoA,tempoB,0.0025*C.BBox_diagonal));
					lc.push_back(col);
					lp.push_back(createSphere(tempoA,0.0025*C.BBox_diagonal));
					lc.push_back(col);
				}
			}*/


			std::vector < Point_d > poin3res;
			for(int k=0; k<(int)poin2res_graham.size();k++){
				FT X1=poin2res_graham[k].x()*vb1.x()+poin2res_graham[k].y()*vb2.x()-plane.d()*vortho.x();
				FT X2=poin2res_graham[k].x()*vb1.y()+poin2res_graham[k].y()*vb2.y()-plane.d()*vortho.y();
				FT X3=poin2res_graham[k].x()*vb1.z()+poin2res_graham[k].y()*vb2.z()-plane.d()*vortho.z();
				Point_d tempo(X1,X2,X3);
				poin3res.push_back(tempo);
			}
			point3dres_graham.push_back(poin3res);
			color_col_graham.push_back(col);

	/*		
			CGAL::Object ch_object;
			Point_d pt_centroide(0,0,0);
			for(int k=0; k<(int)C.plane_point_index[i].size();k++){
				int ind=C.plane_point_index[i][k];
				Point_d pt=C.HPS[ind].position; 
				Point_d ptp=plane.projection(pt);
				C.HPS[ind].position=ptp;
				Point_d ptcen=CGAL::barycenter(pt_centroide,k,ptp,1); pt_centroide=ptcen;
				counting1++;
			}*/
			//vector_centroids_of_extracted_planes.push_back(pt_centroide);
		//	lp_centroid.push_back(createSphere_fine(pt_centroide,0.02*C.BBox_diagonal));
		//	lc_centroid.push_back(col);

		} 

		//cout<<"-> "<<C.extracted_planes.size()<< " planes found ("<<(float)100*counting1/HPS.size()<<" % of points fitted)"<<endl;

	
			char *namo_inliers_colored= "Results_inliers.ply";
			colorplanset2ply(namo_inliers_colored,listpts,vector_of_colors);

			char *namo_graham= "Results_convex_envelops.ply";
			colorplanset2ply(namo_graham,point3dres_graham,color_col_graham);

			char *namo_alphashape= "Results_alpha_shapes.ply";
			colorplanset2ply(namo_alphashape,point3dres_alpha_shape,color_col_alpha_shape);

	//		char *namo2e= "Primitives_alpha_shape_segments.ply";
	//		save_listpolyhedron2ply(lp,namo2e,lc);

	//		char *namo3e= "Primitives_centroids.ply";
	//		save_listpolyhedron2ply(lp_centroid,namo3e,lc_centroid);


return true;
}


template <class Polyhedron>
bool load_polyhedron(Polyhedron& P, std::string filename)
{
std::cout << "----- Loading polyhedron from '" << filename << "'... "; std::cout.flush();

std::ifstream iFileT(filename.c_str(),std::ios::in);
iFileT >> P;

if (!P.is_valid()) return false;

P.normalize_border();
std::cout << P.size_of_vertices() << " vertices, " << P.size_of_halfedges()/2 << " edges, " << P.size_of_border_edges() << " border edges, " << P.size_of_facets() << " facets" << std::endl;

if (!P.is_pure_triangle())
{
std::cerr << "Only pure triangle meshes are currently supported" << std::endl;
return false;
}

return true;
}



template <class Polyhedron>
bool save_polyhedron(const Polyhedron& P, std::string filename)
{
	std::cout << "Saving polyhedron to '" << filename << "'... "; std::cout.flush();
	std::ofstream out(filename.c_str());
	if (!out.is_open()) return false;
	std::string ext = boost::filesystem::extension( boost::filesystem::path(filename) );
	if (ext == ".obj")
		CGAL::print_polyhedron_wavefront(out, P);
	else if (ext == ".off")
		CGAL::print_polyhedron_OFF(out, P);
	if (out.fail()) return false;
	std::cout << P.size_of_vertices() << " vertices, " << P.size_of_halfedges()/2 << " edges, " << P.size_of_border_edges() << " border edges, " << P.size_of_facets() << " facets" << std::endl;
	return true;
}






template <class Polyhedron>
bool save_polyhedron2ply(const Polyhedron& P, char *nom, int choix_col)
{
	typedef typename Polyhedron::Vertex                                 Vertex;
    typedef typename Polyhedron::Vertex_const_iterator                  VCI;
    typedef typename Polyhedron::Facet_const_iterator                   FCI;
    typedef typename Polyhedron::Halfedge_around_facet_const_circulator HFCC;

	std::cout <<"Saving polyhedron to "<<nom<<endl;
	OFSTREAM_TEXTE(fic,nom);
	if(choix_col==0) //avec attribut couleur sur les facettes
	{
		//header
		fic<<"ply"<<endl;
		fic<<"format ascii 1.0"<<endl;
		fic<<"comment author: F. Lafarge"<<endl;
		fic<<"element vertex "<<P.size_of_vertices()<<endl;
		fic<<"property float x"<<endl;
		fic<<"property float y"<<endl;
		fic<<"property float z"<<endl;
		fic<<"element face "<<P.size_of_facets()<<endl;
		fic<<"property list uchar int vertex_index"<<endl;
		fic<<"property uchar red"<<endl;
		fic<<"property uchar green"<<endl;
		fic<<"property uchar blue"<<endl;
		fic<<"end_header"<<endl;

		//vertex list
		for( VCI vi = P.vertices_begin(); vi != P.vertices_end(); ++vi) 
		{
			fic<<vi->point().x()<<" "<<vi->point().y()<<" "<<vi->point().z()<<endl;
		}

		//facet list
		for( FCI fi = P.facets_begin(); fi != P.facets_end(); ++fi)
		{
			HFCC hc = fi->facet_begin();
			HFCC hc_end = hc;
			std::size_t n = circulator_size( hc);
			CGAL_assertion( n >= 3);
			fic<<n<<" ";
			do {
				fic<<hc->vertex()->index()<<" ";
				++hc;
			} while( hc != hc_end);
			fic<<(int)fi->color().r()<<" "<<(int)fi->color().g()<<" "<<(int)fi->color().b()<<endl;
		}
	}
	else //avec attribut couleur sur les vertex
	{


		//header
		fic<<"ply"<<endl;
		fic<<"format ascii 1.0"<<endl;
		fic<<"comment author: F. Lafarge"<<endl;
		fic<<"element vertex "<<P.size_of_vertices()<<endl;
		fic<<"property float x"<<endl;
		fic<<"property float y"<<endl;
		fic<<"property float z"<<endl;
		fic<<"property uchar red"<<endl;
		fic<<"property uchar green"<<endl;
		fic<<"property uchar blue"<<endl;
		fic<<"element face "<<P.size_of_facets()<<endl;
		fic<<"property list uchar int vertex_index"<<endl;
		fic<<"end_header"<<endl;

		//vertex list
		for( VCI vi = P.vertices_begin(); vi != P.vertices_end(); ++vi) 
		{
			fic<<vi->point().x()<<" "<<vi->point().y()<<" "<<vi->point().z()<<" "<<(int)vi->color().r()<<" "<<(int)vi->color().g()<<" "<<(int)vi->color().b()<<endl;
		}

		//facet list
		for( FCI fi = P.facets_begin(); fi != P.facets_end(); ++fi)
		{
			HFCC hc = fi->facet_begin();
			HFCC hc_end = hc;
			std::size_t n = circulator_size( hc);
			CGAL_assertion( n >= 3);
			fic<<n<<" ";
			do {
				fic<<hc->vertex()->index()<<" ";
				++hc;
			} while( hc != hc_end);
			fic<<endl;
			//fic<<(int)fi->color().r()<<" "<<(int)fi->color().g()<<" "<<(int)fi->color().b()<<endl;
		}



	}

fic<<endl;	
std::cout << P.size_of_vertices() << " vertices, " << P.size_of_halfedges()/2 << " edges, " << P.size_of_border_edges() << " border edges, " << P.size_of_facets() << " facets" << std::endl;
return true;
}





#endif
