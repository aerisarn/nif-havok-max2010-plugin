#ifndef __BHKHELPERFUNCS_H__
#define __BHKHELPERFUNCS_H__

extern void CalcAxisAlignedBox(Mesh& mesh, Box3& box);
extern void CalcAxisAlignedBox(Mesh& mesh, Box3& box, Matrix3* tm);
extern void CalcAxisAlignedSphere(Mesh& mesh, Point3& center, float& radius);
extern void CalcCenteredSphere(Mesh& mesh, Point3& center, float& radius);
extern void CalcCapsule(Mesh &mesh, Point3& pt1, Point3& pt2, float& r1, float& r2);
extern void CalcOrientedBox(Mesh &mesh, float& udim, float& vdim, float& ndim, Point3& center, Matrix3& rtm);
extern bool CanCalcCapsule();
extern bool CanCalcOrientedBox();

extern void BuildBox(Mesh&mesh, float l, float w, float h);
extern void BuildSphere(Mesh&mesh, float radius, int segs=32, int smooth=1, float startAng = 0.0f);
extern void BuildCapsule(Mesh &mesh, Point3 pt1, Point3 pt2, float r1, float r2);
extern void BuildScubaMesh(Mesh &mesh, int segs, int smooth, int llsegs, 
						   float radius1, float radius2, float cylh);

extern void InitMaterialTypeCombo(HWND hWnd, int comboid);
extern int GetHavokIndexFromMaterials(/*HavokMaterial*/ int havk_material, /*SkyrimHavokMaterial*/ int skyrim_havok_material);
extern int GetHavokIndexFromMaterial(int havok_material);
extern int GetHavokIndexFromSkyrimMaterial(int skyrim_havok_material);
extern bool GetHavokMaterialsFromIndex(int idx, /*HavokMaterial*/int* havk_material, /*SkyrimHavokMaterial*/int* skyrim_havok_material);
extern int GetEquivalentSkyrimMaterial(int havok_material);

extern void InitLayerTypeCombo(HWND hWnd, int comboid);
extern int GetHavokIndexFromLayers(int havk_layer, int skyrim_havok_layer);
extern int GetHavokIndexFromLayer(int havok_layer);
extern int GetHavokIndexFromSkyrimLayer(int skyrim_havok_layer);
extern bool GetHavokLayersFromIndex(int idx, int* havk_layer, int* skyrim_havok_layer);
extern int GetEquivalentSkyrimLayer(int havok_layer);
#endif