#pragma warning( disable:4800 )

#include <map>
#include "NifProps.h"
#include "NifStrings.h"
#include "NifPlugins.h"
#include "NifGui.h"
#include "meshadj.h"

using namespace std;

#define MAKE_QUAD(na,nb,nc,nd,sm,b) {MakeQuad(nverts,&(mesh.faces[nf]),na, nb, nc, nd, sm, b);nf+=2;}

// Misc stuff
#define MAX_SEGMENTS	200
#define MIN_SEGMENTS	4

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float(1.0E30)

#define MIN_SMOOTH		0
#define MAX_SMOOTH		1

enum 
{
	POSX = 0,	// right
	POSY = 1,	// back
	POSZ = 2,	// top
	NEGX = 3,	// left
	NEGY = 4,	// front
	NEGZ = 5,	// bottom
};

int direction(Point3 *v) {
	Point3 a = v[0]-v[2];
	Point3 b = v[1]-v[0];
	Point3 n = CrossProd(a,b);
	switch(MaxComponent(n)) {
	  case 0: return (n.x<0)?NEGX:POSX;
	  case 1: return (n.y<0)?NEGY:POSY;
	  case 2: return (n.z<0)?NEGZ:POSZ;
	}
	return 0;
}

// Remap the sub-object material numbers so that the top face is the first one
// The order now is:
// Top / Bottom /  Left/ Right / Front / Back
static int mapDir[6] ={ 3, 5, 0, 2, 4, 1 };

// vertices ( a b c d ) are in counter clockwise order when viewd from 
// outside the surface unless bias!=0 in which case they are clockwise
static void MakeQuad(int nverts, Face *f, int a, int b , int c , int d, int sg, int bias) {
	int sm = 1<<sg;
	assert(a<nverts);
	assert(b<nverts);
	assert(c<nverts);
	assert(d<nverts);
	if (bias) {
		f[0].setVerts( b, a, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,0,1);
		f[1].setVerts( d, c, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,0,1);
	} else {
		f[0].setVerts( a, b, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,1,0);
		f[1].setVerts( c, d, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,1,0);
	}
}

void CalcAxisAlignedBox(Mesh& mesh, Box3& box)
{
	int nv = mesh.getNumVerts();
	box.IncludePoints(mesh.getVertPtr(0), nv, nullptr);
}

void CalcAxisAlignedBox(Mesh& mesh, Box3& box, Matrix3 *tm)
{
	int nv = mesh.getNumVerts();
	box.IncludePoints(mesh.getVertPtr(0), nv, tm);
}

// Calculate bounding sphere using minimum-volume axis-align bounding box.  Its fast but not a very good fit.
void CalcAxisAlignedSphere(Mesh& mesh, Point3& center, float& radius)
{
	//--Calculate center & radius--//

	//Set lows and highs to first vertex
	size_t nv = mesh.getNumVerts();

	Point3 lows = mesh.getVert(0);
	Point3 highs = mesh.getVert(0);

	//Iterate through the vertices, adjusting the stored values
	//if a vertex with lower or higher values is found
	for ( size_t i = 0; i < nv; ++i ) {
		const Point3 & v = mesh.getVert(i);

		if ( v.x > highs.x ) highs.x = v.x;
		else if ( v.x < lows.x ) lows.x = v.x;

		if ( v.y > highs.y ) highs.y = v.y;
		else if ( v.y < lows.y ) lows.y = v.y;

		if ( v.z > highs.z ) highs.z = v.z;
		else if ( v.z < lows.z ) lows.z = v.z;
	}

	//Now we know the extent of the shape, so the center will be the average
	//of the lows and highs
	center = (highs + lows) / 2.0f;

	//The radius will be the largest distance from the center
	Point3 diff;
	float dist2(0.0f), maxdist2(0.0f);
	for ( size_t i = 0; i < nv; ++i ) {
		const Point3 & v = mesh.getVert(i);

		diff = center - v;
		dist2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if ( dist2 > maxdist2 ) maxdist2 = dist2;
	};
	radius = sqrt(maxdist2);
}

// Calculate bounding sphere using average position of the points.  Better fit but slower.
void CalcCenteredSphere(Mesh& mesh, Point3& center, float& radius)
{
	int nv = mesh.getNumVerts();
	Point3 sum(0.0f, 0.0f, 0.0f);
	for (int i=0; i<nv; ++i)
		sum += mesh.getVert(i);
	center = sum / float(nv);
	float radsq = 0.0f;
	for (int i=0; i<nv; ++i){
		Point3 diff = mesh.getVert(i) - center;
		float mag = diff.LengthSquared();
		radsq = max(radsq, mag);
	}
	radius = Sqrt(radsq);
}

#define MAKE_QUAD(na,nb,nc,nd,sm,b) {MakeQuad(nverts,&(mesh.faces[nf]),na, nb, nc, nd, sm, b);nf+=2;}

void BuildBox(Mesh&mesh, float l, float w, float h)
{
	int ix,iy,iz,nf,kv,mv,nlayer,topStart,midStart;
	int nverts,nv,nextk,nextm,wsp1;
	int nfaces;
	Point3 va,vb,p;
	BOOL bias = 0;
	const int lsegs = 1, wsegs = 1, hsegs = 1;

	// Start the validity interval at forever and whittle it down.
	if (h<0.0f) bias = 1;

	// Number of verts
	// bottom : (lsegs+1)*(wsegs+1)
	// top	: (lsegs+1)*(wsegs+1)
	// sides	: (2*lsegs+2*wsegs)*(hsegs-1)

	// Number of rectangular faces.
	// bottom : (lsegs)*(wsegs)
	// top	: (lsegs)*(wsegs)
	// sides	: 2*(hsegs*lsegs)+2*(wsegs*lsegs)

	wsp1 = wsegs + 1;
	nlayer	=  2*(lsegs+wsegs);
	topStart = (lsegs+1)*(wsegs+1);
	midStart = 2*topStart;

	nverts = midStart+nlayer*(hsegs-1);
	nfaces = 4*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.InvalidateTopologyCache();

	nv = 0;

	vb =  Point3(w,l,h)/float(2);	
	va = -vb;

	float dx = w/wsegs;
	float dy = l/lsegs;
	float dz = h/hsegs;

	// do bottom vertices.
	p.z = va.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
		}
		p.y += dy;
	}

	nf = 0;

	// do bottom faces.
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1);
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+wsegs+1, kv+wsegs+2, kv+1, 1, bias);
			kv++;
		}
	}
	assert(nf==lsegs*wsegs*2);

	// do top vertices.
	p.z = vb.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
		}
		p.y += dy;
	}

	// do top faces (lsegs*wsegs);
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1)+topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, kv+wsegs+2,kv+wsegs+1, 2, bias);
			kv++;
		}
	}
	assert(nf==lsegs*wsegs*4);

	// do middle vertices 
	for(iz=1; iz<hsegs; iz++) {

		p.z = va.z + dz * iz;

		// front edge
		p.x = va.x;	 p.y = va.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);	 p.x += dx;	}

		// right edge
		p.x = vb.x;	  p.y = va.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);	 p.y += dy;	}

		// back edge
		p.x =  vb.x;  p.y =	 vb.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);	 p.x -= dx;	}

		// left edge
		p.x = va.x;	 p.y =	vb.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);	 p.y -= dy;	}
	}

	if (hsegs==1) {
		// do FRONT faces -----------------------
		kv = 0;
		mv = topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv+1, mv, 3, bias);
			kv++;
			mv++;
		}

		// do RIGHT faces.-----------------------
		kv = wsegs;	 
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+wsp1, mv, 4, bias);
			kv += wsp1;
			mv += wsp1;
		}	

		// do BACK faces.-----------------------
		kv = topStart - 1;
		mv = midStart - 1;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv-1, mv, 5, bias);
			kv --;
			mv --;
		}

		// do LEFT faces.----------------------
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv-wsp1, mv-wsp1, mv, 6, bias);
			kv -= wsp1;
			mv -= wsp1;
		}
	}

	else {
		// do front faces.
		kv = 0;
		mv = midStart;
		for(iz=0; iz<hsegs; iz++) {
			if (iz==hsegs-1) mv = topStart;
			for (ix=0; ix<wsegs; ix++) 
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 3, bias);
			kv = mv;
			mv += nlayer;
		}

		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2);

		// do RIGHT faces.-------------------------
		// RIGHT bottom row:
		kv = wsegs; // into bottom layer. 
		mv = midStart + wsegs; // first layer of mid verts


		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+1, mv, 4, bias);
			kv += wsp1;
			mv ++;
		}

		// RIGHT middle part:
		kv = midStart + wsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				MAKE_QUAD(kv+iy, kv+iy+1, mv+iy+1, mv+iy, 4, bias);
			}
			kv += nlayer;
		}

		// RIGHT top row:
		kv = midStart + wsegs + (hsegs-2)*nlayer; 
		mv = topStart + wsegs;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+1, mv+wsp1, mv, 4, bias);
			mv += wsp1;
			kv++;
		}

		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2 + lsegs*hsegs*2);

		// do BACK faces. ---------------------
		// BACK bottom row:
		kv = topStart - 1;
		mv = midStart + wsegs + lsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv+1, mv, 5, bias);
			kv --;
			mv ++;
		}

		// BACK middle part:
		kv = midStart + wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (ix=0; ix<wsegs; ix++) {
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 5, bias);
			}
			kv += nlayer;
		}

		// BACK top row:
		kv = midStart + wsegs + lsegs + (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1)+wsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv-1, mv, 5, bias);
			mv --;
			kv ++;
		}

		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*4 + lsegs*hsegs*2);

		// do LEFT faces. -----------------
		// LEFT bottom row:
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = midStart + 2*wsegs +lsegs;
		for (iy=0; iy<lsegs; iy++) {
			nextm = mv+1;
			if (iy==lsegs-1) 
				nextm -= nlayer;
			MAKE_QUAD(kv, kv-wsp1, nextm, mv, 6, bias);
			kv -=wsp1;
			mv ++;
		}

		// LEFT middle part:
		kv = midStart + 2*wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				nextm = mv+1;
				nextk = kv+iy+1;
				if (iy==lsegs-1) { 
					nextm -= nlayer;
					nextk -= nlayer;
				}
				MAKE_QUAD(kv+iy, nextk, nextm, mv, 6, bias);
				mv++;
			}
			kv += nlayer;
		}

		// LEFT top row:
		kv = midStart + 2*wsegs + lsegs+ (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1);
		for (iy=0; iy<lsegs; iy++) {
			nextk = kv+1;
			if (iy==lsegs-1) 
				nextk -= nlayer;
			MAKE_QUAD(kv, nextk, mv-wsp1, mv, 6, bias);
			mv -= wsp1;
			kv++;
		}
	}

	mesh.setNumTVerts(0);
	mesh.setNumTVFaces(0);
	for (nf = 0; nf<nfaces; nf++) {
		Face& f = mesh.faces[nf];
		DWORD* nv = f.getAllVerts();
		Point3 v[3];
		for (int ix =0; ix<3; ix++)
			v[ix] = mesh.getVert(nv[ix]);
		int dir = direction(v);
		mesh.setFaceMtlIndex(nf,0);
		//mesh.setFaceMtlIndex(nf,mapDir[dir]);
	}

	mesh.InvalidateTopologyCache();
}

extern void BuildSphere(Mesh&mesh, float radius, int segs, int smooth, float startAng)
{
	Point3 p;	
	int ix,na,nb,nc,nd,jx,kx;
	int nf=0,nv=0;
	float delta, delta2;
	float a,alt,secrad,secang,b,c;
	float hemi = 0.0f;

	LimitValue(segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, MIN_SMOOTH, MAX_SMOOTH);
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);

	float totalPie(0.0f);
	if (hemi>=1.0f) hemi = 0.9999f;
	hemi = (1.0f-hemi) * PI;
	float basedelta=2.0f*PI/(float)segs;
	delta2 = basedelta;
	delta  = basedelta;

	int rows = int(hemi/delta) + 1;
	int realsegs=segs;
	int nverts = rows * realsegs + 2;
	int nfaces = rows * realsegs * 2;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);
	int lastvert=nverts-1;

	// Top vertex 
	mesh.setVert(nv, 0.0f, 0.0f, radius);
	nv++;

	// Middle vertices 
	alt=delta;
	for(ix=1; ix<=rows; ix++) {		
		a = (float)cos(alt)*radius;		
		secrad = (float)sin(alt)*radius;
		secang = startAng; //0.0f
		for(jx=0; jx<segs; ++jx) {
			b = (float)cos(secang)*secrad;
			c = (float)sin(secang)*secrad;
			mesh.setVert(nv++,b,c,a);
			secang+=delta2;
		}
		alt+=delta;		
	}

	/* Bottom vertex */
	mesh.setVert(nv++, 0.0f, 0.0f,-radius);

	// Now make faces 

	BitArray startSliceFaces;
	BitArray endSliceFaces;

	// Make top conic cap
	for(ix=1; ix<=segs; ++ix) {
		nc=(ix==segs)?1:ix+1;
		mesh.faces[nf].setEdgeVisFlags(1,1,1);
		mesh.faces[nf].setSmGroup(smooth?1:0);
		mesh.faces[nf].setMatID(1);
		mesh.faces[nf].setVerts(0, ix, nc);
		nf++;
	}

	/* Make midsection */
	int lastrow=rows-1,lastseg=segs-1,almostlast=lastseg-1;
	for(ix=1; ix<rows; ++ix) {
		jx=(ix-1)*segs+1;
		for(kx=0; kx<segs; ++kx) {
			na = jx+kx;
			nb = na+segs;
			nc = (kx==lastseg)? jx+segs: nb+1;
			nd = (kx==lastseg)? jx : na+1;

			mesh.faces[nf].setEdgeVisFlags(1,1,0);
			mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setMatID(1); 
			mesh.faces[nf].setVerts(na,nb,nc);
			nf++;

			mesh.faces[nf].setEdgeVisFlags(0,1,1);
			mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setMatID(1);
			mesh.faces[nf].setVerts(na,nc,nd);
			nf++;
		}
	}

	// Make bottom conic cap
	na = mesh.getNumVerts()-1;
	int botsegs=segs;
	jx = (rows-1)*segs+1;lastseg=botsegs-1;
	int fstart = nf;
	for(ix=0; ix<botsegs; ++ix) {
		nc = ix + jx;
		nb = (ix==lastseg)?jx:nc+1;
		mesh.faces[nf].setEdgeVisFlags(1,1,1);
		mesh.faces[nf].setSmGroup(smooth?1:0);
		mesh.faces[nf].setMatID(1);
		mesh.faces[nf].setVerts(na, nb, nc);
		nf++;
	}

	mesh.setNumTVerts(0);
	mesh.setNumTVFaces(0);
	mesh.InvalidateTopologyCache();
}



void AddFace(Face *f,int a,int b,int c,int evis,int smooth_group)
{ 
	const int ALLF = 4;
	f[0].setSmGroup(smooth_group);
	f[0].setMatID((MtlID)0); 	 /*default */
	if (evis==0) f[0].setEdgeVisFlags(1,1,0);
	else if (evis==1) f[0].setEdgeVisFlags(0,1,1);
	else if (evis==2) f[0].setEdgeVisFlags(0,0,1);
	else if (evis==ALLF) f[0].setEdgeVisFlags(1,1,1);
	else f[0].setEdgeVisFlags(1,0,1);	
	f[0].setVerts(a,b,c);
}


void BuildScubaMesh(Mesh &mesh, int segs, int smooth, int llsegs, 
					float radius1, float radius2, float cylh)
{
	Point3 p;
	int ix,jx,ic = 1;
	int nf=0,nv=0, capsegs=(int)(segs/2.0f),csegs=0;
	float ang;	
	float startAng = 0.0f;	
	float totalPie = TWOPI;
	int lsegs = llsegs-1 + 2*capsegs;
	int levels=csegs*2+(llsegs-1);
	int capv=segs,sideedge=capsegs+csegs;
	int totlevels=levels+capsegs*2+2;
	int tvinslice=totlevels+totlevels-2;
	float delta = (float)2.0*PI/(float)segs;
	int VertexPerLevel=segs;
	int nfaces=2*segs*(levels+1);
	int ntverts=2*(segs+1)+llsegs-1;
	int *edgelstl=new int[totlevels];
	int *edgelstr=new int[totlevels];
	int lastlevel=totlevels-1,dcapv=capv-1,dvertper=VertexPerLevel-1;
	edgelstr[0] = edgelstl[0] = 0;
	edgelstr[1] = 1;
	edgelstl[1] = capv;
	for (int i=2;i<=sideedge;i++){ 
		edgelstr[i]=edgelstr[i-1]+capv;
		edgelstl[i]=edgelstr[i]+dcapv;
	}
	while ((i<lastlevel)&&(i<=totlevels-sideedge)){ 
		edgelstr[i]=edgelstr[i-1]+VertexPerLevel;
		edgelstl[i]=edgelstr[i]+dcapv;
		i++;
	}
	while (i<lastlevel) { 
		edgelstr[i]=edgelstr[i-1]+capv;
		edgelstl[i]=edgelstr[i]+dcapv;
		i++;
	}
	edgelstl[lastlevel]= (edgelstr[lastlevel]=edgelstl[i-1]+1);
	int nverts=edgelstl[lastlevel]+1;
	nfaces+=2*segs*(2*capsegs-1);

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);
	mesh.setNumTVerts(0);
	mesh.setNumTVFaces(0);
	mesh.setSmoothFlags(smooth != 0);

	// bottom vertex 
	float height = cylh + radius1 + radius2;
	mesh.setVert(nv, Point3(0.0f,0.0f,height));
	mesh.setVert(nverts-1, Point3(0.0f,0.0f,0.0f));		

	// Top (1) and bottom (2) cap vertices
	float ru,cang,sang;
	int msegs=segs,deltaend=nverts-capv-1;
	ang = startAng;	 
	msegs--;
	float rincr=PI/(2.0f*capsegs),aincr;
	for (jx = 0; jx<=msegs; jx++) 
	{
		cang=(float)cos(ang);
		sang=(float)sin(ang);
		for(ix=1; ix<=sideedge; ix++) {
			aincr = (rincr*(float)ix);
			ru=(float)sin(aincr);

			p.x = cang*radius1*ru;
			p.y = sang*radius1*ru;	
			p.z = (jx==0) ? height-radius1*(1.0f-(float)cos(aincr)) : mesh.verts[edgelstr[ix]].z;
			mesh.setVert(edgelstr[ix]+jx, p);

			p.x = cang*radius2*ru;
			p.y = sang*radius2*ru;	
			p.z = (jx==0) ? radius2*(1.0f-(float)cos(aincr)) : mesh.verts[edgelstr[lastlevel-ix]].z ;
			mesh.setVert(edgelstr[lastlevel-ix]+jx,p);
		}
		ang += delta;
	}

	//// Middle vertices 
	//int sidevs,startv=edgelstr[sideedge],deltav;				
	//for(ix=1; ix<llsegs; ix++) {
	//   // Put center vertices all the way up
	//   float   u = float(ix)/float(llsegs);
	//   float rad = (radius1*(1.0f-u) + radius2*u);
	//   p.z = cylh *((float)ix/float(llsegs)) + radius2;
	//   ang = startAng;
	//   for (sidevs=0;sidevs<VertexPerLevel;sidevs++)
	//      p.x = (float)cos(ang)*rad;
	//      p.y = (float)sin(ang)*rad;
	//      mesh.setVert(nv, p);
	//      nv++;
	//      ang += delta;
	//   }	
	//}

	//top layer done, now reflect sides down 
	int sidevs,deltav;
	int startv=edgelstr[sideedge];
	int endv=edgelstr[totlevels-capsegs-1]; 
	if (llsegs>1)
	{
		float sincr = cylh/llsegs;
		for (sidevs=0;sidevs<VertexPerLevel;sidevs++)
		{
			Point3 topp = mesh.verts[startv];
			Point3 botp = mesh.verts[endv];
			p.x = (topp.x + botp.x) /  2.0f;
			p.y = (topp.y + botp.y) /  2.0f;
			deltav=VertexPerLevel;
			for (ic=1;ic<llsegs;ic++)
			{
				p.z = topp.z-sincr*ic;
				mesh.setVert(startv+deltav, p);
				deltav+=VertexPerLevel;
			}
			startv++;
		}
	}
	int lasttvl=0,lasttvr=0;
	int lvert=segs;
	int t0,t1,b0,b1,tvt0=0,tvt1=0,tvb0=1,tvb1=2,fc=0,smoothgr=(smooth?4:0),vseg=segs+1;
	int tvcount=0,lowerside=lastlevel-sideedge,onside=0;

	BOOL ok,wrap;
	// Now make faces ---
	for (int clevel=0;clevel<lastlevel-1;clevel++)
	{
		t1=(t0=edgelstr[clevel])+1;
		b1=(b0=edgelstr[clevel+1])+1;
		ok=1; wrap=FALSE;
		if ((clevel>0)&&(onside==1)) {
			tvt0++;tvt1++;tvb0++,tvb1++;
		}
		if (clevel==1) {
			tvt0=1;tvt1=2;
		}
		if (clevel==sideedge) {
			tvt1+=lvert;tvt0+=lvert;tvb0+=vseg;tvb1+=vseg;onside++;
		} else if (clevel==lowerside) {
			tvt1+=vseg;tvt0+=vseg;tvb0+=lvert;tvb1+=lvert;onside++;
		}
		while ((b0<edgelstl[clevel+1])||ok)
		{
			if (b1==edgelstr[clevel+2]) {
				b1=edgelstr[clevel+1]; 
				t1=edgelstr[clevel];
				ok=FALSE;
				wrap=(onside!=1);
			}
			if (smooth) smoothgr=4;
			AddFace(&mesh.faces[fc++],t0,b0,b1,0,smoothgr);
			if (clevel>0) {
				AddFace(&mesh.faces[fc++],t0,b1,t1,1,smoothgr);
				t0++;t1++;
			}
			b0++;b1++;tvb0++,tvb1++;
		}
	}
	smoothgr=(smooth?4:0);
	t1=(t0=edgelstr[lastlevel-1])+1;b0=edgelstr[lastlevel];
	int lastpt=lastlevel;
	if (onside==1) {
		tvt0++;
		tvt1++;
		tvb0++;
		tvb1++;
	}
	if (sideedge==1) {
		tvt1+=vseg;
		tvt0+=vseg;
		tvb0+=lvert;
		tvb1+=lvert;
		onside++;
	}
	while (t0<edgelstl[lastpt]) {
		if (t1==edgelstr[lastlevel]) {
			t1=edgelstr[lastlevel-1];
			tvt1-=segs;
		}
		AddFace(&mesh.faces[fc++],t0,b0,t1,1,smoothgr);
		t0++;t1++;
	}
	for (i=0;i<nverts;i++) 
		mesh.verts[i].z -= (radius2 + cylh/2.0f);

	if (edgelstr) delete []edgelstr;
	if (edgelstl) delete []edgelstl;
	assert(fc==mesh.numFaces);
	//	assert(nv==mesh.numVerts);
	mesh.InvalidateTopologyCache();
}



extern HINSTANCE hInstance;
class MagicCode
{
private:
	struct Triangle { int a, b, c; };
	typedef int (__stdcall * fnCalcCapsule)
	    (int nverts, const Point3 *verts, 
		Point3& pt1, Point3& pt2, float& r1, float& r2);

	typedef int (__stdcall * fnCalcOrientedBox)
	    (int nverts, const Point3 *verts,
		float& udim, float& vdim, float& ndim,
		Point3& center, Point3& Uaxis, Point3& Vaxis, Point3& Naxis);

	typedef void (__stdcall * fnCalcMassProps)
	(	int nverts, const Point3* verts, 
		int ntris, const Triangle* tris,
		int iBodyCoords, float &rfMass,
		Point3& rkCenter, Matrix3& rkInertia);

	HMODULE hMagicLib;
	fnCalcCapsule CalcCapsule;
	fnCalcOrientedBox CalcOrientedBox;
	fnCalcMassProps CalcMassProps;

public:
	MagicCode() : hMagicLib(0), CalcCapsule(0), CalcOrientedBox(0), CalcMassProps(0) {
	}

	~MagicCode() {
		if (hMagicLib) FreeLibrary(hMagicLib);
	}

	bool Initialize()
	{
		if (hMagicLib == nullptr)
		{
			TCHAR curfile[_MAX_PATH];
			GetModuleFileName(hInstance, curfile, MAX_PATH);
			PathRemoveFileSpec(curfile);
			PathAppend(curfile, TEXT("NifMagic.dll"));
			hMagicLib = LoadLibrary( curfile );
			if (hMagicLib == nullptr)
				hMagicLib = LoadLibrary( TEXT("Nifmagic.dll") );
			CalcCapsule = (fnCalcCapsule)GetProcAddress( hMagicLib, "CalcCapsule" );
			CalcOrientedBox = (fnCalcOrientedBox)GetProcAddress( hMagicLib, "CalcOrientedBox" );
			CalcMassProps = (fnCalcMassProps)GetProcAddress( hMagicLib, "CalcMassProps" );
		}
		// Now returns TRUE if ANY of the desired methods are present.
		// Checks for individual methods will need to check both Initialize() and the appropriate Has...() method.
		return ( nullptr != CalcCapsule || nullptr != CalcOrientedBox || nullptr != CalcMassProps );
	}

	bool HasCalcCapsule() {return nullptr != CalcCapsule;}
	bool HasCalcOrientedBox() {return nullptr != CalcOrientedBox;}
	bool HasCalcMassProps() {return nullptr != CalcMassProps;}
	void DoCalcCapsule(Mesh &mesh, Point3& pt1, Point3& pt2, float& r1, float& r2)
	{
		if (Initialize() && HasCalcCapsule())
		{
			CalcCapsule( mesh.getNumVerts(), &mesh.verts[0], pt1, pt2, r1, r2);
		}
	}

	void DoCalcOrientedBox(Mesh &mesh, float& udim, float& vdim, float& ndim, Point3& center, Matrix3& rtm)
	{
		if (Initialize() && HasCalcOrientedBox())
		{
			Point3 Uaxis, Vaxis, Naxis;
			CalcOrientedBox(mesh.getNumVerts(), &mesh.verts[0], udim, vdim, ndim, center, Uaxis, Vaxis, Naxis);
			rtm.Set(Uaxis, Vaxis, Naxis, center);
		}
	}

	void DoCalcMassProps( Mesh &mesh,
		bool bBodyCoords, float &rfMass,
		Point3& rkCenter, Matrix3& rkInertia)
	{
		if (Initialize() && HasCalcMassProps())
		{
			vector<Triangle> tris;
			tris.resize(mesh.getNumFaces());
			for (int i=0; i<mesh.getNumFaces(); ++i)
			{
				Triangle& tri = tris[i];
				Face& face = mesh.faces[i];
				tri.a = face.getVert(0);
				tri.b = face.getVert(1);
				tri.c = face.getVert(2);
			}
			CalcMassProps( mesh.getNumVerts(), &mesh.verts[0]
				, tris.size(), &tris[0]
				, bBodyCoords ? 1 : 0, rfMass, rkCenter, rkInertia );
		}
	}

} TheMagicCode;

extern bool CanCalcCapsule()
{
	return TheMagicCode.Initialize() && TheMagicCode.HasCalcCapsule();
}
extern bool CanCalcOrientedBox()
{
	return TheMagicCode.Initialize() && TheMagicCode.HasCalcOrientedBox();
}
extern bool CanCalcMassProps()
{
	return TheMagicCode.Initialize() && TheMagicCode.HasCalcMassProps();
}

// Calculate capsule from mesh.  While radii on the endcaps is possible we do 
//   currently calculate then differently.
extern void CalcCapsule(Mesh &mesh, Point3& pt1, Point3& pt2, float& r1, float& r2)
{
	TheMagicCode.DoCalcCapsule(mesh, pt1, pt2, r1, r2);
}

// Calculate OBB (oriented bounding box) from mesh.  Returns each of the 3 dimensions of the box, 
// its center, and the rotation matrix necessary to get the orientation.
extern void CalcOrientedBox(Mesh &mesh, float& udim, float& vdim, float& ndim, Point3& center, Matrix3& rtm)
{
	TheMagicCode.DoCalcOrientedBox(mesh, udim, vdim, ndim, center, rtm);
}

extern void CalcMassProps( Mesh &mesh,
							bool bBodyCoords, float &rfMass,
							Point3& rkCenter, Matrix3& rkInertia)
{
	TheMagicCode.DoCalcMassProps(mesh, bBodyCoords, rfMass, rkCenter, rkInertia);
}

extern void BuildCapsule(Mesh &mesh, Point3 pt1, Point3 pt2, float r1, float r2)
{
	int segs = 12;
	int hsegs = 1;
	int smooth = 1;

	float h = (pt1 - pt2).Length();

	Point3 center = ((pt2 + pt1) / 2.0f);
	Point3 norm = Normalize(pt2 - pt1);
	Matrix3 mat;
	MatrixFromNormal(norm,mat);
	Matrix3 newTM = mat * TransMatrix(center);

	// Build capsule to suggested size
	BuildScubaMesh(mesh, segs, smooth, hsegs, r1, r2, h);

	// Reorient the capsule.
	MNMesh mn(mesh);
	Matrix3 tm(true);
	tm.Translate(center);
	mn.Transform(newTM);
	mn.OutToTri(mesh);
}

using namespace Niflib;

typedef struct HavokEnumLookupType { // : EnumLookupType
	int value;
	const TCHAR *name;
	int havok;
	int skyrimHavok;
} HavokEnumLookupType;

extern const HavokEnumLookupType MaterialTypes[] = {
	{ 0, TEXT("Stone (O,F3,S)"), HAV_MAT_STONE, SKY_HAV_MAT_STONE },
	{ 1, TEXT("Cloth (O,F3,S)"), HAV_MAT_CLOTH, SKY_HAV_MAT_CLOTH },
	{ 2, TEXT("Dirt (O,F3,S)"), HAV_MAT_DIRT, SKY_HAV_MAT_DIRT },
	{ 3, TEXT("Glass (O,F3,S)"), HAV_MAT_GLASS, SKY_HAV_MAT_GLASS },
	{ 4, TEXT("Grass (O,F3,S)"), HAV_MAT_GRASS, SKY_HAV_MAT_GRASS },
	{ 5, TEXT("Metal (O,F3,S)"), HAV_MAT_METAL, SKY_HAV_MAT_SOLID_METAL },
	{ 6, TEXT("Organic (O,F3,S)"), HAV_MAT_ORGANIC, SKY_HAV_MAT_ORGANIC },
	{ 7, TEXT("Skin (O,F3,S)"), HAV_MAT_SKIN, SKY_HAV_MAT_SKIN },
	{ 8, TEXT("Water (O,F3,S)"), HAV_MAT_WATER, SKY_HAV_MAT_WATER },
	{ 9, TEXT("Wood (O,F3,S)"), HAV_MAT_WOOD, SKY_HAV_MAT_WOOD },
	{ 10, TEXT("Heavy Stone (O,F3,S)"), HAV_MAT_HEAVY_STONE, SKY_HAV_MAT_HEAVY_STONE },
	{ 11, TEXT("Heavy Metal (O,F3,S)"), HAV_MAT_HEAVY_METAL, SKY_HAV_MAT_HEAVY_METAL },
	{ 12, TEXT("Heavy Wood (O,F3,S)"), HAV_MAT_HEAVY_WOOD, SKY_HAV_MAT_HEAVY_WOOD },
	{ 13, TEXT("Chain (O,F3,S)"), HAV_MAT_CHAIN, SKY_HAV_MAT_MATERIAL_CHAIN },
	{ 14, TEXT("Snow (O,F3,S)"), HAV_MAT_SNOW, SKY_HAV_MAT_SNOW },
	{ 15, TEXT("Stone Stairs (O,F3,S)"), HAV_MAT_STONE_STAIRS, SKY_HAV_MAT_STAIRS_BROKEN_STONE },
	{ 16, TEXT("Cloth Stairs (O,F3)"), HAV_MAT_CLOTH_STAIRS, -1 },
	{ 17, TEXT("Dirt Stairs (O,F3)"), HAV_MAT_DIRT_STAIRS, -1 },
	{ 18, TEXT("Glass Stairs (O,F3)"), HAV_MAT_GLASS_STAIRS, -1 },
	{ 19, TEXT("Grass Stairs (O,F3)"), HAV_MAT_GRASS_STAIRS, -1 },
	{ 20, TEXT("Metal Stairs (O,F3)"), HAV_MAT_METAL_STAIRS, -1 },
	{ 21, TEXT("Organic Stairs (O,F3)"), HAV_MAT_ORGANIC_STAIRS, -1 },
	{ 22, TEXT("Skin Stairs (O,F3)"), HAV_MAT_SKIN_STAIRS, -1 },
	{ 23, TEXT("Water Stairs (O,F3)"), HAV_MAT_WATER_STAIRS, -1 },
	{ 24, TEXT("Wood Stairs (O,F3,S)"), HAV_MAT_WOOD_STAIRS, SKY_HAV_MAT_STAIRS_WOOD },
	{ 25, TEXT("Heavy Stone Stairs (O,F3,S)"), HAV_MAT_HEAVY_STONE_STAIRS, SKY_HAV_MAT_MATERIAL_STONE_AS_STAIRS },
	{ 26, TEXT("Heavy Metal Stairs (O,F3)"), HAV_MAT_HEAVY_METAL_STAIRS, -1 },
	{ 27, TEXT("Heavy Wood Stairs (O,F3,S)"), HAV_MAT_HEAVY_WOOD_STAIRS, SKY_HAV_MAT_MATERIAL_WOOD_AS_STAIRS },
	{ 28, TEXT("Chain Stairs (O,F3)"), HAV_MAT_CHAIN_STAIRS, -1 },
	{ 29, TEXT("Snow Stairs (O,F3,S)"), HAV_MAT_SNOW_STAIRS, SKY_HAV_MAT_STAIRS_SNOW },
	{ 30, TEXT("Elevator (O,F3)"), HAV_MAT_ELEVATOR, -1 },
	{ 31, TEXT("Rubber (O,F3)"), HAV_MAT_RUBBER, -1 },
	{ 32, TEXT("Barrel (S)"), -1, SKY_HAV_MAT_BARREL },
	{ 33, TEXT("Bottle (S)"), -1, SKY_HAV_MAT_BOTTLE },
	{ 34, TEXT("Broken Stone (S)"), -1, SKY_HAV_MAT_BROKEN_STONE },
	{ 35, TEXT("Dragon (S)"), -1, SKY_HAV_MAT_DRAGON },
	{ 36, TEXT("Gravel (S)"), -1, SKY_HAV_MAT_GRAVEL },
	{ 37, TEXT("Ice (S)"), -1, SKY_HAV_MAT_ICE },
	{ 38, TEXT("Light Wood (S)"), -1, SKY_HAV_MAT_LIGHT_WOOD },
	{ 39, TEXT("Armor Heavy (S)"), -1, SKY_HAV_MAT_MATERIAL_ARMOR_HEAVY },
	{ 40, TEXT("Armor Light (S)"), -1, SKY_HAV_MAT_MATERIAL_ARMOR_LIGHT },
	{ 41, TEXT("Arrow (S)"), -1, SKY_HAV_MAT_MATERIAL_ARROW },
	{ 42, TEXT("Axe 1Hand (S)"), -1, SKY_HAV_MAT_MATERIAL_AXE_1HAND },
	{ 43, TEXT("Basket (S)"), -1, SKY_HAV_MAT_MATERIAL_BASKET },
	{ 44, TEXT("Blade 1 Hand (S)"), -1, SKY_HAV_MAT_MATERIAL_BLADE_1HAND },
	{ 45, TEXT("Blade 1Hand Small (S)"), -1, SKY_HAV_MAT_MATERIAL_BLADE_1HAND_SMALL },
	{ 46, TEXT("Blade 2Hand (S)"), -1, SKY_HAV_MAT_MATERIAL_BLADE_2HAND },
	{ 47, TEXT("Blunt 2Hand (S)"), -1, SKY_HAV_MAT_MATERIAL_BLUNT_2HAND },
	{ 48, TEXT("Bone (S)"), -1, SKY_HAV_MAT_MATERIAL_BONE },
	{ 49, TEXT("Book (S)"), -1, SKY_HAV_MAT_MATERIAL_BOOK },
	{ 50, TEXT("Bottle Small (S)"), -1, SKY_HAV_MAT_MATERIAL_BOTTLE_SMALL },
	{ 51, TEXT("Boulder Large (S)"), -1, SKY_HAV_MAT_MATERIAL_BOULDER_LARGE },
	{ 52, TEXT("Boulder Medium (S)"), -1, SKY_HAV_MAT_MATERIAL_BOULDER_MEDIUM },
	{ 53, TEXT("Boulder Small (S)"), -1, SKY_HAV_MAT_MATERIAL_BOULDER_SMALL },
	{ 54, TEXT("Bows Staves (S)"), -1, SKY_HAV_MAT_MATERIAL_BOWS_STAVES },
	{ 55, TEXT("Carpet (S)"), -1, SKY_HAV_MAT_MATERIAL_CARPET },
	{ 56, TEXT("Ceramic Medium (S)"), -1, SKY_HAV_MAT_MATERIAL_CERAMIC_MEDIUM },
	{ 57, TEXT("Chain Metal (S)"), -1, SKY_HAV_MAT_MATERIAL_CHAIN_METAL },
	{ 58, TEXT("Coin (S)"), -1, SKY_HAV_MAT_MATERIAL_COIN },
	{ 59, TEXT("Shield Heavy (S)"), -1, SKY_HAV_MAT_MATERIAL_SHIELD_HEAVY },
	{ 60, TEXT("Shield Light (S)"), -1, SKY_HAV_MAT_MATERIAL_SHIELD_LIGHT },
	{ 61, TEXT("Skin Large (S)"), -1, SKY_HAV_MAT_MATERIAL_SKIN_LARGE },
	{ 62, TEXT("Skin Small (S)"), -1, SKY_HAV_MAT_MATERIAL_SKIN_SMALL },
	{ 63, TEXT("Mud (S)"), -1, SKY_HAV_MAT_MUD },
	{ 64, TEXT("Sand (S)"), -1, SKY_HAV_MAT_SAND },
	{ -1, NULL },
};
//extern const EnumLookupType *enumMaterialTypes = (EnumLookupType *)MaterialTypes;

void InitMaterialTypeCombo(HWND hWnd, int comboid)
{
	SendDlgItemMessage(hWnd, comboid, CB_ADDSTRING, 0, LPARAM(TEXT("<Default>")));
	for (const HavokEnumLookupType* flag = MaterialTypes; flag->name != NULL; ++flag) {
		SendDlgItemMessage(hWnd, comboid, CB_ADDSTRING, 0, LPARAM(flag->name));
	}	
}

bool GetHavokMaterialsFromIndex(int idx, /*HavokMaterial*/int* havk_material, /*SkyrimHavokMaterial*/int* skyrim_havok_material)
{
	int offset = idx - 1; // adjust to combo indexes (includes Default)
	if (idx >= 0 && idx < _countof(MaterialTypes))
	{
		if (havk_material) *havk_material = MaterialTypes[idx].havok;
		if (skyrim_havok_material) *skyrim_havok_material = MaterialTypes[idx].skyrimHavok;
		return true;
	}
	if (havk_material) *havk_material = HAV_MAT_STONE;
	if (skyrim_havok_material) *skyrim_havok_material = SkyrimHavokMaterial::SKY_HAV_MAT_STONE;
	return false;
}

int GetHavokIndexFromMaterials(/*HavokMaterial*/ int havk_material, /*SkyrimHavokMaterial*/ int skyrim_havok_material)
{
	for (const HavokEnumLookupType* flag = MaterialTypes; flag->name != NULL; ++flag) {
		if (skyrim_havok_material != NP_INVALID_HVK_MATERIAL) {
			if (skyrim_havok_material == flag->skyrimHavok)
				return flag->value; 
		} else if (havk_material != NP_INVALID_HVK_MATERIAL) {
			if (havk_material == flag->havok)
				return flag->value;
		}
	}
	return NP_INVALID_HVK_MATERIAL;
}

int GetHavokIndexFromMaterial(int havok_material)
{
	for (auto flag = MaterialTypes; flag->name != NULL; ++flag) {
		if (havok_material == flag->havok)
			return flag->value; 
	}
	return NP_INVALID_HVK_MATERIAL;
}

int GetHavokIndexFromSkyrimMaterial(int skyrim_havok_material)
{
	for (auto flag = MaterialTypes; flag->name != NULL; ++flag) {
		if (skyrim_havok_material == flag->skyrimHavok)
			return flag->value;
	}
	return NP_INVALID_HVK_MATERIAL;
}


extern int GetEquivalentSkyrimMaterial(int havok_material)
{
	for (const HavokEnumLookupType* flag = MaterialTypes; flag->name != NULL; ++flag) {
		if (havok_material == flag->havok)
			return flag->skyrimHavok;
	}
	return HAV_MAT_STONE;
}

extern const HavokEnumLookupType LayerTypes[] = {
	{ 0, TEXT("Unidentified (O,F3,S)"), OL_UNIDENTIFIED, SKYL_UNIDENTIFIED },
	{ 1, TEXT("Static (O,F3,S)"), OL_STATIC, SKYL_STATIC },
	{ 2, TEXT("AnimStatic (O,F3,S)"), OL_ANIM_STATIC, SKYL_ANIMSTATIC },
	{ 3, TEXT("Transparent (O,F3,S)"), OL_TRANSPARENT, SKYL_TRANSPARENT },
	{ 4, TEXT("Clutter (O,F3,S)"), OL_CLUTTER, SKYL_CLUTTER },
	{ 5, TEXT("Weapon (O,F3,S)"), OL_WEAPON, SKYL_WEAPON },
	{ 6, TEXT("Projectile (O,F3,S)"), OL_PROJECTILE, SKYL_PROJECTILE },
	{ 7, TEXT("Spell (O,F3,S)"), OL_SPELL, SKYL_SPELL },
	{ 8, TEXT("Biped (O,F3,S)"), OL_BIPED, SKYL_BIPED },
	{ 9, TEXT("Tree (O,F3,S)"), OL_TREES, SKYL_TREES },
	{ 10, TEXT("Prop (O,F3,S)"), OL_PROPS, SKYL_PROPS },
	{ 11, TEXT("Water (O,F3,S)"), OL_WATER, SKYL_WATER },
	{ 12, TEXT("Trigger (O,F3,S)"), OL_TRIGGER, SKYL_TRIGGER },
	{ 13, TEXT("Terrain (O,F3,S)"), OL_TERRAIN, SKYL_TERRAIN },
	{ 14, TEXT("Trap (O,F3,S)"), OL_TRAP, SKYL_TRAP },
	{ 15, TEXT("NonCollidable (O,F3,S)"), OL_NONCOLLIDABLE, SKYL_NONCOLLIDABLE },
	{ 16, TEXT("CloudTrap (O,F3,S)"), OL_CLOUD_TRAP, SKYL_CLOUD_TRAP },
	{ 17, TEXT("Ground (O,F3,S)"), OL_GROUND, SKYL_GROUND },
	{ 18, TEXT("Portal (O,F3,S)"), OL_PORTAL, SKYL_PORTAL },
	{ 19, TEXT("Stairs (O,F3)"), OL_STAIRS, -1 },
	{ 20, TEXT("CharController (O,F3)"), OL_CHAR_CONTROLLER, -1 },
	{ 21, TEXT("AvoidBox (O,F3)"), OL_AVOID_BOX, -1 },
	{ 22, TEXT("? (O,F3)"), OL_UNKNOWN1, -1 },
	{ 23, TEXT("? (O,F3)"), OL_UNKNOWN1, -1 },
	{ 24, TEXT("CameraPick (O,F3)"), OL_CAMERA_PICK, -1 },
	{ 25, TEXT("ItemPick (O,F3)"), OL_ITEM_PICK, -1 },
	{ 26, TEXT("LineOfSight (O,F3)"), OL_LINE_OF_SIGHT, -1 },
	{ 27, TEXT("PathPick (O,F3)"), OL_PATH_PICK, -1 },
	{ 28, TEXT("CustomPick1 (O,F3)"), OL_CUSTOM_PICK_1, -1 },
	{ 29, TEXT("CustomPick2 (O,F3)"), OL_CUSTOM_PICK_2, -1 },
	{ 30, TEXT("SpellExplosion (O,F3)"), OL_SPELL_EXPLOSION, -1 },
	{ 31, TEXT("DroppingPick (O,F3)"), OL_DROPPING_PICK, -1 },
	{ 32, TEXT("Other (O,F3)"), OL_OTHER, -1 },
	{ 33, TEXT("Head (O,F3)"), OL_HEAD, -1 },
	{ 34, TEXT("Body (O,F3)"), OL_BODY, -1 },
	{ 35, TEXT("Spine1 (O,F3)"), OL_SPINE1, -1 },
	{ 36, TEXT("Spine2 (O,F3)"), OL_SPINE2, -1 },
	{ 37, TEXT("LUpperArm (O,F3)"), OL_L_UPPER_ARM, -1 },
	{ 38, TEXT("LForeArm (O,F3)"), OL_L_FOREARM, -1 },
	{ 39, TEXT("LHand (O,F3)"), OL_L_HAND, -1 },
	{ 40, TEXT("LThigh (O,F3)"), OL_L_THIGH, -1 },
	{ 41, TEXT("LCalf (O,F3)"), OL_L_CALF, -1 },
	{ 42, TEXT("LFoot (O,F3)"), OL_L_FOOT, -1 },
	{ 43, TEXT("RUpperArm (O,F3)"), OL_R_UPPER_ARM, -1 },
	{ 44, TEXT("RForeArm (O,F3)"), OL_R_FOREARM, -1 },
	{ 45, TEXT("RHand (O,F3)"), OL_R_HAND, -1 },
	{ 46, TEXT("RThigh (O,F3)"), OL_R_THIGH, -1 },
	{ 47, TEXT("RCalf (O,F3)"), OL_R_CALF, -1 },
	{ 48, TEXT("RFoot (O,F3)"), OL_R_FOOT, -1 },
	{ 49, TEXT("Tail (O,F3)"), OL_TAIL, -1 },
	{ 50, TEXT("SideWeapon (O,F3)"), OL_SIDE_WEAPON, -1 },
	{ 51, TEXT("Shield (O,F3)"), OL_SHIELD, -1 },
	{ 52, TEXT("Quiver (O,F3)"), OL_QUIVER, -1 },
	{ 53, TEXT("BackWeapon (O,F3)"), OL_BACK_WEAPON, -1 },
	{ 54, TEXT("BackWeapon (O,F3)"), OL_BACK_WEAPON, -1 },
	{ 55, TEXT("PonyTail (O,F3)"), OL_PONYTAIL, -1 },
	{ 56, TEXT("Wing (O,F3)"), OL_WING, -1 },
	{ 57, TEXT("Null (O,F3,S)"), OL_NULL, SKYL_NULL },
	{ 58, TEXT("Debris Small (S)"), -1, SKYL_DEBRIS_SMALL },
	{ 59, TEXT("Debris Large (S)"), -1, SKYL_DEBRIS_LARGE },
	{ 60, TEXT("Acoustic Space (S)"), -1, SKYL_ACOUSTIC_SPACE },
	{ 61, TEXT("Actor Zone (S)"), -1, SKYL_ACTORZONE },
	{ 62, TEXT("Projectile Zone (S)"), -1, SKYL_PROJECTILEZONE },
	{ 63, TEXT("Gas Trap (S)"), -1, SKYL_GASTRAP },
	{ 64, TEXT("Shell Casing (S)"), -1, SKYL_SHELLCASING },
	{ 65, TEXT("Transparent Small (S)"), -1, SKYL_TRANSPARENT_SMALL },
	{ 66, TEXT("Invisible Wall (S)"), -1, SKYL_INVISIBLE_WALL },
	{ 67, TEXT("Transparent Small Anim (S)"), -1, SKYL_TRANSPARENT_SMALL_ANIM },
	{ 68, TEXT("Ward (S)"), -1, SKYL_WARD },
	{ 69, TEXT("Char Controller (S)"), -1, SKYL_CHARCONTROLLER },
	{ 70, TEXT("Stair Helper (S)"), -1, SKYL_STAIRHELPER },
	{ 71, TEXT("Dead Bip (S)"), -1, SKYL_DEADBIP },
	{ 72, TEXT("Biped No CC (S)"), -1, SKYL_BIPED_NO_CC },
	{ 73, TEXT("Avoid Box (S)"), -1, SKYL_AVOIDBOX },
	{ 74, TEXT("Collision Box (S)"), -1, SKYL_COLLISIONBOX },
	{ 75, TEXT("Camera Sphere (S)"), -1, SKYL_CAMERASHPERE },
	{ 76, TEXT("Door Detection (S)"), -1, SKYL_DOORDETECTION },
	{ 77, TEXT("Cone Projectile (S)"), -1, SKYL_CONEPROJECTILE },
	{ 78, TEXT("Camera Pick (S)"), -1, SKYL_CAMERAPICK },
	{ 79, TEXT("Item Pick (S)"), -1, SKYL_ITEMPICK },
	{ 80, TEXT("Line of Sight (S)"), -1, SKYL_LINEOFSIGHT },
	{ 81, TEXT("Path Pick (S)"), -1, SKYL_PATHPICK },
	{ 82, TEXT("Custom Pick 1 (S)"), -1, SKYL_CUSTOMPICK1 },
	{ 83, TEXT("Custom Pick 2 (S)"), -1, SKYL_CUSTOMPICK2 },
	{ 84, TEXT("Spell Explosion (S)"), -1, SKYL_SPELLEXPLOSION },
	{ 85, TEXT("Dropping Pick (S)"), -1, SKYL_DROPPINGPICK },
};

void InitLayerTypeCombo(HWND hWnd, int comboid)
{
	//SendDlgItemMessage(hWnd, comboid, CB_ADDSTRING, 0, LPARAM(TEXT("<Default>")));
	for (const HavokEnumLookupType* flag = LayerTypes; flag->name != NULL; ++flag) {
		SendDlgItemMessage(hWnd, comboid, CB_ADDSTRING, 0, LPARAM(flag->name));
	}
}

bool GetHavokLayersFromIndex(int idx, /*HavokLayer*/int* havok_Layer, /*SkyrimHavokLayer*/int* skyrim_havok_layer)
{
	int offset = idx; // adjust to combo indexes (includes Default)
	if (idx >= 0 && idx < _countof(LayerTypes))
	{
		if (havok_Layer) *havok_Layer = LayerTypes[idx].havok;
		if (skyrim_havok_layer) *skyrim_havok_layer = LayerTypes[idx].skyrimHavok;
		return true;
	}
	if (havok_Layer) *havok_Layer = 0;
	if (skyrim_havok_layer) *skyrim_havok_layer = 0;
	return false;
}

int GetHavokIndexFromLayers(/*HavokLayer*/ int havok_layer, /*SkyrimHavokLayer*/ int skyrim_havok_layer)
{
	for (const HavokEnumLookupType* flag = LayerTypes; flag->name != NULL; ++flag) {
		if (skyrim_havok_layer != -1) {
			if (skyrim_havok_layer == flag->skyrimHavok)
				return flag->value; 
		}
		else if (havok_layer != -1) {
			if (havok_layer == flag->havok)
				return flag->value; 
		}
	}
	return 0;
}

int GetHavokIndexFromLayer(int havok_Layer)
{
	for (auto flag = LayerTypes; flag->name != NULL; ++flag) {
		if (havok_Layer == flag->havok)
			return flag->value; // adjust to combo indexes
	}
	return 0;
}

int GetHavokIndexFromSkyrimLayer(int skyrim_havok_layer)
{
	for (auto flag = LayerTypes; flag->name != NULL; ++flag) {
		if (skyrim_havok_layer == flag->skyrimHavok)
			return flag->value; // adjust to combo indexes
	}
	return 0;
}


extern int GetEquivalentSkyrimLayer(int havok_Layer)
{
	for (const HavokEnumLookupType* flag = LayerTypes; flag->name != NULL; ++flag) {
		if (havok_Layer == flag->havok)
			return flag->skyrimHavok;
	}
	return 0;
}
