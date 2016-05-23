/*
	NodeSplinePath.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodeSplinePath.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSplinePath, NodeTransform );
REGISTER_FACTORY_KEY(  NodeSplinePath, 4015895435786920966LL );

BEGIN_PROPERTY_LIST( NodeSplinePath, NodeTransform )
	ADD_PROPERTY( m_Points );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeSplinePath::NodeSplinePath()
{}

//-------------------------------------------------------------------------------

void NodeSplinePath::preRender( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	// call the base class
	NodeTransform::preRender( context, frame, position );
}

//-------------------------------------------------------------------------------

int NodeSplinePath::pointCount() const
{
	return( m_Points.size() );
}

const Vector3 & NodeSplinePath::point( int n ) const
{
	return( m_Points[ n ] );
}

void NodeSplinePath::setPath( const Array< Vector3 > & points )
{
	m_Points = points;
}

//-------------------------------------------------------------------------------
// EOF


/*
grabbed this from: http://www.mhri.edu.au/~pdb/

// This returns the point "output" on the spline curve.
// The parameter "v" indicates the position, it ranges from 0 to n-t+2

void SplinePoint(u,n,t,v,control,output)
int *u,n,t;
double v;
XYZ *control,*output;
{
   int k;
   double b;

   output->x = 0;
   output->y = 0;
   output->z = 0;

   for (k=0;k<=n;k++) {
      b = SplineBlend(k,t,u,v);
      output->x += control[k].x * b;
      output->y += control[k].y * b;
      output->z += control[k].z * b;
   }
}

//   Calculate the blending value, this is done recursively.
   
//   If the numerator and denominator are 0 the expression is 0.
//   If the deonimator is 0 the expression is 0

double SplineBlend(k,t,u,v)
int k,t,*u;
double v;
{
   double value;

   if (t == 1) {
      if ((u[k] <= v) && (v < u[k+1]))
         value = 1;
      else
         value = 0;
   } else {
      if ((u[k+t-1] == u[k]) && (u[k+t] == u[k+1]))
         value = 0;
      else if (u[k+t-1] == u[k]) 
         value = (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
      else if (u[k+t] == u[k+1])
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v);
     else
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v) + 
                 (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
   }
   return(value);
}


//   The positions of the subintervals of v and breakpoints, the position
//   on the curve are called knots. Breakpoints can be uniformly defined
//   by setting u[j] = j, a more useful series of breakpoints are defined
//   by the function below. This set of breakpoints localises changes to
//   the vicinity of the control point being modified.

void SplineKnots(u,n,t)
int *u,n,t;
{
   int j;

   for (j=0;j<=n+t;j++) {
      if (j < t)
         u[j] = 0;
      else if (j <= n)
         u[j] = j - t + 1;
      else if (j > n)
         u[j] = n - t + 2;	
   }
}


//   Create all the points along a spline curve
//   Control points "inp", "n" of them.
//   Knots "knots", degree "t".
//   Ouput curve "outp", "res" of them.

void SplineCurve(inp,n,knots,t,outp,res)
XYZ *inp;
int n;
int *knots;
int t;
XYZ *outp;
int res;
{
   int i;
   double interval,increment;

   interval = 0;
   increment = (n - t + 2) / (double)(res - 1);
   for (i=0;i<res-1;i++) {
      SplinePoint(knots,n,t,interval,inp,&(outp[i]));
      interval += increment;
   }
   outp[res-1] = inp[n];
}

//   Example of how to call the spline functions
//	Basically one needs to create the control points, then compute
//   the knot positions, then calculate points along the curve.

#define N 3
XYZ inp[N+1] = {0.0,0.0,0.0,   1.0,0.0,3.0,   2.0,0.0,1.0,   4.0,0.0,4.0};
#define T 3
int knots[N+T+1];
#define RESOLUTION 200
XYZ outp[RESOLUTION];

int main(argc,argv)
int argc;
char **argv;
{
   int i;

   SplineKnots(knots,N,T);
   SplineCurve(inp,N,knots,T,outp,RESOLUTION);

   // Display the curve, in this case in OOGL format for GeomView 
   printf("LIST\n");
   printf("{ = SKEL\n");
   printf("%d %d\n",RESOLUTION,RESOLUTION-1);
   for (i=0;i<RESOLUTION;i++)
      printf("%g %g %g\n",outp[i].x,outp[i].y,outp[i].z);
   for (i=0;i<RESOLUTION-1;i++)
      printf("2 %d %d 1 1 1 1\n",i,i+1);
   printf("}\n");

   // The axes 
   printf("{ = SKEL 3 2  0 0 4  0 0 0  4 0 0  2 0 1 0 0 1 1 2 1 2 0 0 1 1 }\n");

   // Control point polygon 
   printf("{ = SKEL\n");
   printf("%d %d\n",N+1,N);
   for (i=0;i<=N;i++)
      printf("%g %g %g\n",inp[i].x,inp[i].y,inp[i].z);
   for(i=0;i<N;i++)
      printf("2 %d %d 0 1 0 1\n",i,i+1);
   printf("}\n");

}
*/


