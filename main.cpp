/* Instruction de lancement du programme

- argv[1]: adresse relative des donnees a charger
Ex: ../../../data/p3.off
- argv[2]: methode d'evaluation des facettes pour le regroupement de primitives
Parmis: NORMALS, PRIMFIT, PRIMFIT2
- argv[3]: critere de regroupement des facettes
Parmis: - NONE          aucun regroupement de facettes				detection de primitives
        - COPL			regroupement de primitives coplanaires		detection de primitives -> regroupement des prim. coplanaires
		- PARA			regroupement de primitives paralleles		detection de primitives -> regroupement des prim. paralleles entre elles
		- PARA_ORTH		regroupement de primitives orth || para		detection de primitives -> regroupement des prim. para ou orth entre elles
- argv[4]: parametres des methodes employees
STD pour des parametres standards (bon resultats avec la methode NORMALS)

TODO: Transformer cette instruction en verbose, a activer lorsque l'utilisateur rentre comme argv[1] "HELP"
*/

#include <iostream>
#include <vector>
#include <cstring>

#include "types.h"
#include "cloud_segmentation.h"
#include "Visualization_Tools.h"

int main(int argc, char **argv)
{
	cout << "------------------- Segmentation de maillage -------------------" << endl;
	cout << "------ Simon Lebastard --- Pierre-Marie Boulvard----------------" << endl;
	cout << "------------------------ 12 Juin 2016 --------------------------" << endl;
	cout << "_____________________________________________________" << endl << endl;

// -------  Loading data and structure initialization -------//
// ----------------------------------------------------------//

// ------- Show help if argv[1]='HELP' ------- //
if (argv[1] == "HELP")
{
	init_verbose();
	return 0;
}

Cloud_segmentation C;
if( !load_polyhedron( C.mesh , argv[1]) )  return 0;
C.init();
// ----------------------------------------------------------//
// ----------------------------------------------------------//

bool redo=true;

while(redo){

	string params;
	if (argv[4])
		params = argv[4];

	double beta, gamma;
	if (params == "STD")
	{
		beta = 0.5;
		gamma = 0.5;
	}
	else
	{
		cout << endl << "Donner une valeur a beta: ";
		std::cin >> beta;
		cout << endl << "Donner une valeur a gamma: ";
		std::cin >> gamma;
		cout << endl;
	}

	//Labeling
	C.labeling(beta, gamma);
	C.save_result();

	cout<<endl<<"Relaunch ? (yes=1, no=0): ";
	std::cin>>redo;
}

if (argv[2])
	redo = true;

while (redo){

	double max_angle, max_angle_merge, min_size, max_dist;
	int min_facets;
	bool regularize = false;
	double scale = 0;
	string method = argv[2];
	string cluster_mode;
	if (argv[3])
		cluster_mode = argv[3];
	else
		cluster_mode = "NONE";
	while (method != "NORMALS" && method != "DYN_NORMALS" && method != "PRIMFIT" && method != "PRIMFIT2")
	{
		cout << endl << "Preciser la methode d'evaluation a employer (NORMALS, DYN_NORMALS or PRIMFIT): ";
		std::cin >> method;
	}
	if (method=="PRIMFIT2")
	{
		do
		{
			cout << endl << "Give space scale parameter for primitive fitting: ";
			std::cin >> scale;
		} while (scale < 0);
	}
	while (cluster_mode != "NONE" && cluster_mode != "COPL" && cluster_mode != "PARA" && cluster_mode != "PARA_ORTH")
	{
		cout << endl << "Preciser les characteristiques sur lesquelles rassembler les primitives" << endl;
		cout << "(NONE, COPL, PARA or PARA_ORTH) : ";
		std::cin >> cluster_mode;
	} 
	string params ;
	if (argv[4])
		params = argv[4];
	if (params == "STD")
	{
		max_angle = 0.26;
		max_angle_merge = 0.5;
		min_size = 1000;
		min_facets = 60;
	}
	else
	{
		cout << endl << "Donner une valeur a max angle: ";
		std::cin >> max_angle;
		cout << endl << "Donner une valeur a max angle, merging: ";
		std::cin >> max_angle_merge;
		cout << endl << "Donner une aire min par primitive: ";
		std::cin >> min_size;
		cout << endl << "Donner un nb mini de facettes par primitive: ";
		std::cin >> min_facets;
		cout << endl << "Donner une valeur a la distance de proj maximale: ";
		std::cin >> max_dist;
		cout << endl;
	}
	string regul_input;
	if (argv[5])
		regul_input = argv[5];
	if (regul_input == "REG")
		regularize = true;

	//Plane extraction
	C.plane_extraction(max_angle, max_angle_merge, min_size, scale, method, cluster_mode, min_facets, regularize);
	C.save_result2();

	cout << endl << "Relaunch ? (yes=1, no=0): ";
	std::cin >> redo;
}

cout << endl<< "END" << endl;

return 0;
}