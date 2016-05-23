/*
	Chopper.cpp

	This class is used to convert a triangle mesh into triangle strips
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Render3D/Chopper.h"

//----------------------------------------------------------------------------

class Vertex;	
class Triangle 
{
public:
	// Construction
					Triangle(Vertex *v0,Vertex *v1,Vertex *v2);
					~Triangle();
	
	// Mutators
	void            ComputeNormal();
	void            ReplaceVertex(Vertex *vold,Vertex *vnew);
	int             HasVertex(Vertex *v);

	// Data
	Vertex *        vertex[3]; // the 3 points that make this tri
	Vector3         normal;    // unit vector othogonal to this face
};

class Vertex 
{
public:
	// Construction
	                Vertex(Vector3 v,int _id);
	                ~Vertex();
	// Mutators
	void            RemoveIfNonNeighbor(Vertex *n);

	// Data
  	Vector3         position; // location of point in euclidean space
	int             id;       // place of vertex in original list

	Array<Vertex *>  
					neighbor; // adjacent vertices
	Array<Triangle *> 
					face;     // adjacent triangles
	float           objdist;  // cached cost of collapsing edge
	Vertex *        collapse; // candidate vertex for collapse
};

//----------------------------------------------------------------------------

static Array<Triangle *> s_Triangles;
static Array<Vertex *>   s_Vertices;

//----------------------------------------------------------------------------

Triangle::Triangle(Vertex *v0,Vertex *v1,Vertex *v2)
{
	ASSERT(v0!=v1 && v1!=v2 && v2!=v0);

	vertex[0]=v0;
	vertex[1]=v1;
	vertex[2]=v2;

	ComputeNormal();
	s_Triangles.push(this);

	for(int i=0;i<3;i++) 
	{
		vertex[i]->face.push(this);
		for(int j=0;j<3;j++) 
			if(i != j) 
				vertex[i]->neighbor.pushUnique( vertex[j] );
	}
}

Triangle::~Triangle()
{
	s_Triangles.removeSearch(this);

	int i;
	for(i=0;i<3;i++) 
		if(vertex[i]) 
			vertex[i]->face.removeSearch(this);

	for(i=0;i<3;i++) 
	{
		int i2 = (i+1)%3;
		if(!vertex[i] || !vertex[i2]) 
			continue;

		vertex[i ]->RemoveIfNonNeighbor(vertex[i2]);
		vertex[i2]->RemoveIfNonNeighbor(vertex[i ]);
	}
}

int Triangle::HasVertex(Vertex *v) 
{
	return (v==vertex[0] ||v==vertex[1] || v==vertex[2]);
}

void Triangle::ComputeNormal()
{
	Vector3 v0 = vertex[0]->position;
	Vector3 v1 = vertex[1]->position;
	Vector3 v2 = vertex[2]->position;

	//normal = (v0-v1) & (v0-v2);
	normal = (v1-v0) & (v2-v1);
	normal.normalize();
}

void Triangle::ReplaceVertex(Vertex *vold,Vertex *vnew) 
{
	ASSERT(vold && vnew);
	ASSERT(vold==vertex[0] || vold==vertex[1] || vold==vertex[2]);
	ASSERT(vnew!=vertex[0] && vnew!=vertex[1] && vnew!=vertex[2]);

	if( vold==vertex[0] )
		vertex[0]=vnew;
	else if( vold==vertex[1] )
		vertex[1]=vnew;
	else 
		vertex[2]=vnew;

	vold->face.removeSearch(this);

	ASSERT(vnew->face.search(this) < 0);
	vnew->face.push(this);

	int i =0;
	for(;i<3;i++) 
	{
		vold->RemoveIfNonNeighbor(vertex[i]);
		vertex[i]->RemoveIfNonNeighbor(vold);
	}
	for(i=0;i<3;i++) 
	{
		ASSERT( vertex[i]->face.search(this) >= 0 );
		for(int j=0;j<3;j++) 
			if(i!=j) 
				vertex[i]->neighbor.pushUnique( vertex[j] );
	}

	ComputeNormal();
}


//----------------------------------------------------------------------------

Vertex::Vertex(Vector3 v,int _id) 
{
	position = v;
	id=_id;

	s_Vertices.push(this);
}

Vertex::~Vertex()
{
	ASSERT( face.size() == 0 );
	while(neighbor.size() > 0) 
	{
		neighbor[0]->neighbor.removeSearch(this);
		neighbor.removeSearch(neighbor[0]);
	}
	s_Vertices.removeSearch(this);
}

void Vertex::RemoveIfNonNeighbor(Vertex *n) 
{
	// removes n from neighbor list if n isn't a neighbor.
	if(neighbor.search(n) < 0) 
		return;
	for(int i=0;i<face.size();i++) 
	{
		if(face[i]->HasVertex(n)) 
			return;
	}
	neighbor.removeSearch(n);
}

//----------------------------------------------------------------------------

static float ComputeEdgeCollapseCost(Vertex *u,Vertex *v) 
{
	// if we collapse edge uv by moving u to v then how 
	// much different will the model change, i.e. how much "error".
	// Texture, vertex normal, and border vertex code was removed
	// to keep this demo as simple as possible.
	// The method of determining cost was designed in order 
	// to exploit small and coplanar regions for
	// effective polygon reduction.
	// Is is possible to add some checks here to see if "folds"
	// would be generated.  i.e. normal of a remaining face gets
	// flipped.  I never seemed to run into this problem and
	// therefore never added code to detect this case.
	int i;
	float edgelength = (v->position - u->position).magnitude();
	float curvature=0;

	// find the "sides" s_Triangles that are on the edge uv
	Array<Triangle *> sides;
	for(i=0;i<u->face.size();i++) {
		if(u->face[i]->HasVertex(v)){
			sides.push(u->face[i]);
		}
	}
	// use the triangle facing most away from the sides 
	// to determine our curvature term
	for(i=0;i<u->face.size();i++) {
		float mincurv=1; // curve for face i and closer side to it
		for(int j=0;j<sides.size();j++) {
			// use dot product of face normals. '|' defined in vector
			float dotprod = u->face[i]->normal | sides[j]->normal;
			mincurv = Min(mincurv,(1-dotprod)/2.0f);
		}
		curvature = Max(curvature,mincurv);
	}
	// the more coplanar the lower the curvature term   
	return edgelength * curvature;
}

static void ComputeEdgeCostAtVertex(Vertex *v) 
{
	// compute the edge collapse cost for all edges that start
	// from vertex v.  Since we are only interested in reducing
	// the object by selecting the min cost edge at each step, we
	// only cache the cost of the least cost edge at this vertex
	// (in member variable collapse) as well as the value of the 
	// cost (in member variable objdist).
	if(v->neighbor.size()==0) {
		// v doesn't have neighbors so it costs nothing to collapse
		v->collapse=NULL;
		v->objdist=-0.01f;
		return;
	}
	v->objdist = 1000000;
	v->collapse=NULL;
	// search all neighboring edges for "least cost" edge
	for(int i=0;i<v->neighbor.size();i++) 
	{
		float dist;
		dist = ComputeEdgeCollapseCost(v,v->neighbor[i]);
		if(dist<v->objdist) {
			v->collapse=v->neighbor[i];  // candidate for edge collapse
			v->objdist=dist;             // cost of the collapse
		}
	}
}

static void ComputeAllEdgeCollapseCosts() 
{
	// For all the edges, compute the difference it would make
	// to the model if it was collapsed.  The least of these
	// per vertex is cached in each vertex object.
	for(int i=0;i<s_Vertices.size();i++) 
		ComputeEdgeCostAtVertex( s_Vertices[i] );
}

static void Collapse(Vertex *u,Vertex *v)
{
	// Collapse the edge uv by moving vertex u onto v
	// Actually remove tris on uv, then update tris that
	// have u to have v, and then remove u.
	if(!v) {
		// u is a vertex all by itself so just delete it
		delete u;
		return;
	}
	int i;
	Array<Vertex *>tmp;
	// make tmp a list of all the neighbors of u
	for(i=0;i<u->neighbor.size();i++) {
		tmp.push(u->neighbor[i]);
	}
	// delete triangles on edge uv:
	for(i=u->face.size()-1;i>=0;i--) 
	{
		if(u->face[i]->HasVertex(v)) 
		{
			delete(u->face[i]);
		}
	}
	// update remaining triangles to have v instead of u
	for(i=u->face.size()-1;i>=0;i--) 
		u->face[i]->ReplaceVertex(u,v);
	delete u; 

	// recompute the edge collapse costs for neighboring s_Vertices
	for(i=0;i<tmp.size();i++) 
	{
		ComputeEdgeCostAtVertex(tmp[i]);
	}
}

static void AddVertex( Array<Vector3> &vert)
{
	for(int i=0;i<vert.size();i++) 
		Vertex *v = new Vertex(vert[i],i);
}

static void AddFaces( Array<Chopper::Triangle> &tri)
{
	for(int i=0;i<tri.size();i++) 
	{
		if ( tri[i].v[0] == tri[i].v[1] || tri[i].v[1] == tri[i].v[2] || tri[i].v[2] == tri[i].v[0] )
			continue;
		new Triangle( s_Vertices[tri[i].v[0]], s_Vertices[tri[i].v[1]], s_Vertices[tri[i].v[2]] );
	}
}

static Vertex *MinimumCostEdge()
{
	// Find the edge that when collapsed will affect model the least.
	// This funtion actually returns a Vertex, the second vertex
	// of the edge (collapse candidate) is stored in the vertex data.
	// Serious optimization opportunity here: this function currently
	// does a sequential search through an unsorted list :-(
	// Our algorithm could be O(n*lg(n)) instead of O(n*n)
	Vertex *mn=s_Vertices[0];
	for(int i=0;i<s_Vertices.size();i++) 
	{
		if(s_Vertices[i]->objdist < mn->objdist) 
			mn = s_Vertices[i];
	}
	return mn;
}

void Chopper::chop( Array< Vector3 > & vert, Array< Triangle > &tri, 
							Array< int > &map, Array< int > &permutation )
{
	AddVertex(vert);  // put input data into our data structures
	AddFaces(tri);

	ComputeAllEdgeCollapseCosts(); // cache all edge collapse costs

	permutation.allocate(s_Vertices.size());  // allocate space
	map.allocate(s_Vertices.size());          // allocate space

	// reduce the object down to nothing:
	while(s_Vertices.size() > 0) 
	{
		// get the next vertex to collapse
		Vertex *mn = MinimumCostEdge();
		// keep track of this vertex, i.e. the collapse ordering
		permutation[mn->id]=s_Vertices.size()-1;
		// keep track of vertex to which we collapse to
		map[s_Vertices.size()-1] = (mn->collapse)?mn->collapse->id:-1;
		// Collapse this edge
		Collapse(mn,mn->collapse);
	}
	// reorder the map list based on the collapse ordering
	for(int i=0;i<map.size();i++) {
		map[i] = (map[i]==-1)?0:permutation[map[i]];
	}
	// The caller of this function should reorder their vertices
	// according to the returned "permutation".
}

//----------------------------------------------------------------------------
// EOF
