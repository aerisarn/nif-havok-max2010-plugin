////////////////////////////////////////////////////////////
//
//	Shader UI Utility routines
//

#ifndef SHADERUTIL_H
#define SHADERUTIL_H

//#include "ICompositeShader.h"
//#include "expmtlControl.h"

#define ALPHA_MIN	0.015f
#define ALPHA_MAX	0.5f
#define SPEC_MAX	0.5f

#define ALPHA_SZ	(ALPHA_MAX - ALPHA_MIN)

#define DEFAULT_GLOSS2	0.03f	
#define DEFAULT_K_REFL	1.0f	

#define MIN_ORIENT		-999.99	
#define MAX_ORIENT		999.99	

void CombineComponentsAdd( IllumParams& ip );

#define TRANSP_SUB		0
#define TRANSP_ADD		1
#define TRANSP_FILTER	2

Color transpColor( ULONG flags, float opac, Color& filt, Color& diff );

Color OrenNayarIllum( Point3& N, Point3& L, Point3& V, float rough, Color& rho, float* pDiffIntensOut, float NL = -2.0f );

float GaussHighlight( float gloss, float aniso, float orient,  
					  Point3& N, Point3& V, Point3& L, Point3& U, float* pNL );

Point3 GetTangent( ShadeContext &sc, int uvChan );

float GaussHiliteCurve2( float x, float y, float sLevel, float gloss, float aniso );

BOOL IsButtonChecked(HWND hWnd,int id);
void CheckButton(HWND hWnd,int id, BOOL check);

void SetupLockButton(HWND hWnd,int id, BOOL check);
void SetupPadLockButton(HWND hWnd,int id, BOOL check);
void LoadStdShaderResources();

// 2D version
void DrawHilite(HDC hdc, Rect& rect, Shader* pShader );
LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

// anisotropic version, w layers: layer 0= ignore layers, 1=layer1, 2=layer2
void DrawHilite2(HDC hdc, Rect& rect, Shader* pShader, int layer=0 );
LRESULT CALLBACK Hilite2WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Hilite2Layer1WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Hilite2Layer2WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
void UpdateHilite( HWND hwHilite, Shader* pShader, int layer=0 );
void UpdateHilite2( HWND hwHilite, Shader* pShader, int layer=0 );


Color GetMtlColor( int i, Shader* pShader ) ;
TCHAR *GetColorName( int i );
//void SetMtlColor(int i, Color c, StdShader* pShader, ColorSwatch* cs);
void SetMtlColor(int i, Color c, Shader* pShader, IColorSwatch** cs, TimeValue t);

inline float PcToFrac(int pc) { return (float)pc/100.0f; }

inline int FracToPc(float f) {
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
}

// Soften polynomials
#define Soften	 SoftSpline2

// Quadratic
static inline float SoftSpline2(float r) {
	return r*(2.0f-r);
	}

// Cubic
static inline float SoftSpline3(float r) {
	return r*r*(3.0f-2.0f*r);
	}

extern float const Pi;
extern float const Pi2;

// General math inlines
inline float Sqr( float x ) { return x * x; }
inline float Cube( float x ) { return x * x * x; }
inline float Abs( float a ) { return (a < 0.0f) ? -a : a; }
inline void  MinMax( float& a, float& b ) { if (a > b){ float tmp=a; a=b; b=tmp;} }
inline float Dot( Point3& a, Point3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float Len2( Point3& a ) { return Dot(a,a); }
inline float Len( Point3& a ) { return float( sqrt( Len2(a))); }
inline Point3 Normalize( Point3& a ) { float d = Len(a); return d==0.0f ? Point3(0,0,1) : a*(1.0f/d); }
inline float AngleBetween( Point3& a, Point3& b ) { 
	return float( acos( Dot(a,b)/(Len(a)*Len(b)) ));
}
inline float Min( float a, float b ) { return (a < b) ? a : b; }
inline float Min( float a, float b, float c ) { return (a < b) ? Min(a,c) : Min(b,c); }
inline float Min( Color& c ){ return Min( c.r, c.g, c.b ); }
inline float Max( float a, float b ) { return (a < b) ? b : a; }
inline float Max( float a, float b, float c ) { return (a < b) ? Max( b, c ) : Max(a,c); }
inline float Max( Color& c ){ return Max( c.r, c.g, c.b ); }

inline float NormClr( Color& a ){ float m = Max( a ); if(m !=0.0f) a /= m; return m; }

inline float Bound0_1( float x ){ return x < 0.0f ? 0.0f : ( x > 1.0f ? 1.0f : x); }
inline float Bound( float x, float min = 0.0f, float max = 1.0f ){ return x < min? min:( x > max? max : x); }
inline float UBound( float x, float max = 1.0f ){ return x > max ? max : x; }
inline float LBound( float x, float min = 0.0f ){ return x < min ? min : x; }
inline Color Bound( Color& c )
	{ return Color( Bound(c.r), Bound(c.g), Bound(c.b) ); }

inline Color UBound( Color& c, float max = 1.0f )
	{ return Color( UBound(c.r,max), UBound(c.g,max), UBound(c.b,max) ); }
inline Color LBound( Color& c, float min = 0.0f )
	{ return Color( LBound(c.r, min), LBound(c.g, min), LBound(c.b, min) ); }

//inline Point3 Cross( Point3 v0, Point3 v1 ){ return CrossProd( v0, v1 ); }


inline float DegToRdn( float d ){ return d * (1.0f/180.0f) * Pi; } // d/360*2*pi
inline float RdnToDeg( float r ){ return r * 180.0f * (1.0f/Pi); } // r/2pi*360

Point3 RotateVec( Point3& p, Point3& axis, float rdn );





// IDs for all the ParamBlocks and their parameters.  One block UI per rollout.
enum { fos_shader, fos_mtl, fos_bgsm, fos_bgem, };  // pblock IDs
																			   // fos_shader param IDs
enum
{
	fos_shader_type, fos_name, fos_filename,
};
// fos_extended param IDs
enum {
fos_tileu, fos_tilev, fos_uoffset, fos_voffset, fos_uscale, fos_vscale,
fos_alpha, fos_alphablendmode, fos_blendstate, fos_blendfunc1, fos_blendfunc2,
fos_alphatestref, fos_alphatest, fos_zbufferwrite, fos_zbuffertest,
fos_screenspacereflections, fos_wetnesscontrolscreenspacereflections,
fos_decal, fos_twosided, fos_decalnofade, fos_nonoccluder,
fos_refraction, fos_refractionfalloff, fos_refractionpower,
fos_environmentmapping, fos_environmentmappingmaskscale, fos_grayscaletopalettecolor,
};

// fos_sampling param IDs
enum
{
	fos_diffusetexture, fos_normaltexture, fos_smoothspectexture, fos_greyscaletexture,
	fos_envmaptexture, fos_glowtexture, fos_innerlayertexture, fos_wrinklestexture,
	fos_displacementtexture, fos_enableeditoralpharef, fos_rimlighting, fos_rimpower,
	fos_backlightpower, fos_subsurfacelighting, fos_subsurfacelightingrolloff,
	fos_specularenabled, fos_specularcolor, fos_specularmult, fos_smoothness,
	fos_fresnelpower, fos_wetnesscontrolspecscale, fos_wetnesscontrolspecpowerscale,
	fos_wetnesscontrolspecminvar, fos_wetnesscontrolenvmapscale, fos_wetnesscontrolfresnelpower,
	fos_wetnesscontrolmetalness, fos_rootmaterialpath, fos_anisolighting, fos_emitenabled,
	fos_emittancecolor, fos_emittancemult, fos_modelspacenormals, fos_externalemittance,
	fos_backlighting, fos_receiveshadows, fos_hidesecret, fos_castshadows,
	fos_dissolvefade, fos_assumeshadowmask, fos_glowmap, fos_environmentmappingwindow,
	fos_environmentmappingeye, fos_hair, fos_hairtintcolor, fos_tree, fos_facegen,
	fos_skintint, fos_tessellate, fos_displacementtexturebias, fos_displacementtexturescale,
	fos_tessellationpnscale, fos_tessellationbasefactor, fos_tessellationfadedistance,
	fos_grayscaletopalettescale, fos_skewspecularalpha,
};
// fos_dynamics param IDs
enum
{
fos_basetexture, fos_grayscaletexture, fos_envmaptexture2, fos_normaltexture2,
fos_envmapmasktexture, fos_bloodenabled, fos_effectlightingenabled, fos_falloffenabled,
fos_falloffcolorenabled,fos_grayscaletopalettealpha, fos_softenabled,fos_basecolor,
fos_basecolorscale,  fos_falloffstartangle, fos_falloffstopangle, fos_falloffstartopacity,
fos_falloffstopopacity, fos_lightinginfluence, fos_envmapminlod, fos_softdepth, 
};


#endif