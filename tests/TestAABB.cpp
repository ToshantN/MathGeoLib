#include <stdio.h>
#include <stdlib.h>

#include "MathGeoLib.h"
#include "myassert.h"

LCG rng(Clock::TickU32());

#define SCALE 1e2f

AABB RandomAABBContainingPoint(const float3 &pt, float maxSideLength)
{
	float w = rng.Float(0, maxSideLength);
	float h = rng.Float(0, maxSideLength);
	float d = rng.Float(0, maxSideLength);

	AABB a(float3(0,0,0), float3(w,h,d));
	w = rng.Float(0, w);
	h = rng.Float(0, h);
	d = rng.Float(0, d);
	a.Translate(pt - float3(w,h,d));
	assert(!a.IsDegenerate());
	assert(a.IsFinite());
	assert(a.Contains(pt));
	return a;
}

OBB RandomOBBContainingPoint(const float3 &pt, float maxSideLength)
{
	AABB a = RandomAABBContainingPoint(pt, maxSideLength);
	float3x4 rot = float3x4::RandomRotation(rng);
	float3x4 tm = float3x4::Translate(pt) * rot * float3x4::Translate(-pt);
	OBB o = a.Transform(tm);
	assert(!o.IsDegenerate());
	assert(o.IsFinite());
	assert(o.Contains(pt));
	return o;
}

Sphere RandomSphereContainingPoint(const float3 &pt, float maxRadius)
{
	Sphere s(pt, rng.Float(0.001f, maxRadius));
	s.pos += float3::RandomSphere(rng, float3::zero, s.r);
	assert(s.IsFinite());
	assert(!s.IsDegenerate());
	assert(s.Contains(pt));
	return s;
}

Frustum RandomFrustumContainingPoint(const float3 &pt)
{
	Frustum f;
	if (rng.Int(0,1))
	{
		f.type = PerspectiveFrustum;
		f.orthographicWidth = rng.Float(0.001f, SCALE);
		f.orthographicHeight = rng.Float(0.001f, SCALE);
	}
	else
	{
		f.type = OrthographicFrustum;
		f.horizontalFov = rng.Float(0.001f, pi-0.001f);
		f.verticalFov = rng.Float(0.001f, pi-0.001f);
	}
	f.nearPlaneDistance = rng.Float(0.1f, SCALE);
	f.farPlaneDistance = f.nearPlaneDistance + rng.Float(0.1f, SCALE);
	f.pos = float3::zero;
	f.front = float3::RandomDir(rng);
	f.up = f.front.RandomPerpendicular(rng);

	float3 pt2 = f.UniformRandomPointInside(rng);
	f.pos += pt - pt2;

	assert(f.IsFinite());
//	assert(!f.IsDegenerate());
	assert(f.Contains(pt));
	return f;
}

Line RandomLineContainingPoint(const float3 &pt)
{
	float3 dir = float3::RandomDir(rng);
	Line l(pt, dir);
	l.pos = l.GetPoint(rng.Float(-SCALE, SCALE));
	assert(l.IsFinite());
	assert(l.Contains(pt));
	return l;
}

Ray RandomRayContainingPoint(const float3 &pt)
{
	float3 dir = float3::RandomDir(rng);
	Ray l(pt, dir);
	l.pos = l.GetPoint(rng.Float(-SCALE, 0));
	assert(l.IsFinite());
	assert(l.Contains(pt));
	return l;
}

LineSegment RandomLineSegmentContainingPoint(const float3 &pt)
{
	float3 dir = float3::RandomDir(rng);
	float a = rng.Float(0, SCALE);
	float b = rng.Float(0, SCALE);
	LineSegment l(pt + a*dir, pt - b*dir);
	assert(l.IsFinite());
	assert(l.Contains(pt));
	return l;
}

Capsule RandomCapsuleContainingPoint(const float3 &pt)
{
	float3 dir = float3::RandomDir(rng);
	float a = rng.Float(0, SCALE);
	float b = rng.Float(0, SCALE);
	float r = rng.Float(0.001f, SCALE);
	Capsule c(pt + a*dir, pt - b*dir, r);
	float3 d = float3::RandomSphere(rng, float3::zero, c.r);
	c.l.a += d;
	c.l.b += d;
	assert(c.IsFinite());
	assert(c.Contains(pt));

	return c;
}

Plane RandomPlaneContainingPoint(const float3 &pt)
{
	float3 dir = float3::RandomDir(rng);
	Plane p(pt, dir);
	assert(!p.IsDegenerate());
	return p;
}

Triangle RandomTriangleContainingPoint(const float3 &pt)
{
	Plane p = RandomPlaneContainingPoint(pt);
	float3 a = pt;
	float3 b = p.Point(rng.Float(-SCALE, SCALE), rng.Float(-SCALE, SCALE));
	float3 c = p.Point(rng.Float(-SCALE, SCALE), rng.Float(-SCALE, SCALE));
	Triangle t(a,b,c);
	assert(t.Contains(pt));
	/*
	float3 d = t.RandomPointInside(rng);
	float3 br = t.BarycentricUVW(d);
	assert(t.Contains(d));
	d = d - t.a;
	t.a -= d;
	t.b -= d;
	t.c -= d;
	*/
	assert(t.IsFinite());
	assert(!t.IsDegenerate());
	assert(t.Contains(pt));
	return t;
}

Polyhedron RandomPolyhedronContainingPoint(const float3 &pt)
{
	switch(rng.Int(0,7))
	{
	case 0: return RandomAABBContainingPoint(pt, SCALE).ToPolyhedron();
	case 1: return RandomOBBContainingPoint(pt, SCALE).ToPolyhedron();
	case 2: return RandomFrustumContainingPoint(pt).ToPolyhedron();
	case 3: return Polyhedron::Tetrahedron(pt, SCALE); break;
	case 4: return Polyhedron::Octahedron(pt, SCALE); break;
	case 5: return Polyhedron::Hexahedron(pt, SCALE); break;
	case 6: return Polyhedron::Icosahedron(pt, SCALE); break;
	default: return Polyhedron::Dodecahedron(pt, SCALE); break;
	}

//	assert(t.IsFinite());
//	assert(!t.IsDegenerate());
//	assert(t.Contains(pt));
}

Polygon RandomPolygonContainingPoint(const float3 &pt)
{
	Polyhedron p = RandomPolyhedronContainingPoint(pt);
	Polygon poly = p.FacePolygon(rng.Int(0, p.NumFaces()-1));

	float3 pt2 = poly.FastRandomPointInside(rng);
	assert(poly.Contains(pt2));
	poly.Translate(pt - pt2);

	assert(!poly.IsDegenerate());
	assert(!poly.IsNull());
	assert(poly.IsPlanar());
	assert(poly.IsFinite());
	assert(poly.Contains(pt));

	return poly;
}

void TestAABBAABBIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	AABB b = RandomAABBContainingPoint(pt, 10.f);
	assert(a.Intersects(b));
//	assert(a.Distance(b) == 0.f);
}

void TestAABBOBBIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	OBB b = RandomOBBContainingPoint(pt, 10.f);
	assert(a.Intersects(b));
}

void TestAABBLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBPlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBSphereIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Sphere b = RandomSphereContainingPoint(pt, SCALE);
	assert(a.Intersects(b));
}

void TestAABBCapsuleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Capsule b = RandomCapsuleContainingPoint(pt);
	assert(a.Intersects(b));
}
void TestAABBTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBFrustumIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Frustum b = RandomFrustumContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBPolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestAABBPolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	AABB a = RandomAABBContainingPoint(pt, 10.f);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}




void TestOBBOBBIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	OBB b = RandomOBBContainingPoint(pt, 10.f);
	assert(a.Intersects(b));
}

void TestOBBLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBPlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBSphereIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Sphere b = RandomSphereContainingPoint(pt, SCALE);
	assert(a.Intersects(b));
}

void TestOBBCapsuleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Capsule b = RandomCapsuleContainingPoint(pt);
	assert(a.Intersects(b));
}
void TestOBBTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBFrustumIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Frustum b = RandomFrustumContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBPolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestOBBPolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	OBB a = RandomOBBContainingPoint(pt, 10.f);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}





void TestSphereSphereIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Sphere b = RandomSphereContainingPoint(pt, 10.f);
	assert(a.Intersects(b));
}

void TestSphereLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSphereRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSphereLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSpherePlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSphereCapsuleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Capsule b = RandomCapsuleContainingPoint(pt);
	assert(a.Intersects(b));
}
void TestSphereTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSphereFrustumIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Frustum b = RandomFrustumContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSpherePolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestSpherePolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Sphere a = RandomSphereContainingPoint(pt, 10.f);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}




void TestFrustumLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumPlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumCapsuleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Capsule b = RandomCapsuleContainingPoint(pt);
	assert(a.Intersects(b));
}
void TestFrustumTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumFrustumIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Frustum b = RandomFrustumContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumPolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestFrustumPolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Frustum a = RandomFrustumContainingPoint(pt);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}




void TestCapsuleLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsuleRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsuleLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsulePlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsuleCapsuleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Capsule b = RandomCapsuleContainingPoint(pt);
	assert(a.Intersects(b));
}
void TestCapsuleTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsulePolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestCapsulePolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Capsule a = RandomCapsuleContainingPoint(pt);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}





void TestPolyhedronLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronPlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronPolyhedronIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Polyhedron b = RandomPolyhedronContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolyhedronPolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polyhedron a = RandomPolyhedronContainingPoint(pt);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}



void TestPolygonLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolygonRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolygonLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolygonPlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolygonTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPolygonPolygonIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Polygon a = RandomPolygonContainingPoint(pt);
	Polygon b = RandomPolygonContainingPoint(pt);
	assert(a.Intersects(b));
}



void TestTriangleLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Triangle a = RandomTriangleContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestTriangleRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Triangle a = RandomTriangleContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestTriangleLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Triangle a = RandomTriangleContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestTrianglePlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Triangle a = RandomTriangleContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestTriangleTriangleIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Triangle a = RandomTriangleContainingPoint(pt);
	Triangle b = RandomTriangleContainingPoint(pt);
	assert(a.Intersects(b));
}




void TestPlaneLineIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Plane a = RandomPlaneContainingPoint(pt);
	Line b = RandomLineContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPlaneRayIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Plane a = RandomPlaneContainingPoint(pt);
	Ray b = RandomRayContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPlaneLineSegmentIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Plane a = RandomPlaneContainingPoint(pt);
	LineSegment b = RandomLineSegmentContainingPoint(pt);
	assert(a.Intersects(b));
}

void TestPlanePlaneIntersect()
{
	float3 pt = float3::RandomBox(rng, -float3(SCALE,SCALE,SCALE), float3(SCALE,SCALE,SCALE));
	Plane a = RandomPlaneContainingPoint(pt);
	Plane b = RandomPlaneContainingPoint(pt);
	assert(a.Intersects(b));
}

typedef void (*TestFunctionPtr)();
struct Test
{
	std::string name;
	std::string description;
	TestFunctionPtr function;
};

std::vector<Test> tests;

void AddTest(std::string name, TestFunctionPtr function, std::string description = "")
{
	Test t;
	t.name = name;
	t.description = description;
	t.function = function;
	tests.push_back(t);
}

void RunTests(int numTimes)
{
	int numTestsPassed = 0;

	for(size_t i = 0; i < tests.size(); ++i)
	{
		printf("Testing '%s': ", tests[i].name.c_str());
		int numFails = 0;
		int numPasses = 0;
		try
		{
			for(int j = 0; j < numTimes; ++j)
			{
				tests[i].function();
				++numPasses;
			}
		}
		catch(const std::exception &e)
		{
			printf("FAILED: '%s' (%d passes)\n", e.what(), numPasses);
			++numFails;
		}

		if (numFails == 0)
		{
			printf("ok (%d passes)\n", numPasses);
			++numTestsPassed;
		}
	}

	printf("Done. %d tests run. %d passed. %d failed.\n", (int)tests.size(), numTestsPassed, (tests.size() - numTestsPassed));
}

int main()
{
	AddTest("AABB-Line positive intersection", TestAABBLineIntersect);
	AddTest("AABB-Ray positive intersection", TestAABBRayIntersect);
	AddTest("AABB-LineSegment positive intersection", TestAABBLineSegmentIntersect);
	AddTest("AABB-AABB positive intersection", TestAABBAABBIntersect);
	AddTest("AABB-OBB positive intersection", TestAABBOBBIntersect);
	AddTest("AABB-Plane positive intersection", TestAABBPlaneIntersect);
	AddTest("AABB-Sphere positive intersection", TestAABBSphereIntersect);
	AddTest("AABB-Triangle positive intersection", TestAABBTriangleIntersect);
	AddTest("AABB-Capsule positive intersection", TestAABBCapsuleIntersect);
	AddTest("AABB-Frustum positive intersection", TestAABBFrustumIntersect);
	AddTest("AABB-Polygon positive intersection", TestAABBPolygonIntersect);
	AddTest("AABB-Polyhedron positive intersection", TestAABBPolyhedronIntersect);

	AddTest("OBB-Line positive intersection", TestOBBLineIntersect);
	AddTest("OBB-Ray positive intersection", TestOBBRayIntersect);
	AddTest("OBB-LineSegment positive intersection", TestOBBLineSegmentIntersect);
	AddTest("OBB-OBB positive intersection", TestOBBOBBIntersect);
	AddTest("OBB-Plane positive intersection", TestOBBPlaneIntersect);
	AddTest("OBB-Sphere positive intersection", TestOBBSphereIntersect);
	AddTest("OBB-Triangle positive intersection", TestOBBTriangleIntersect);
	AddTest("OBB-Capsule positive intersection", TestOBBCapsuleIntersect);
	AddTest("OBB-Frustum positive intersection", TestOBBFrustumIntersect);
	AddTest("OBB-Polygon positive intersection", TestOBBPolygonIntersect);
	AddTest("OBB-Polyhedron positive intersection", TestOBBPolyhedronIntersect);

	AddTest("Sphere-Line positive intersection", TestSphereLineIntersect);
	AddTest("Sphere-Ray positive intersection", TestSphereRayIntersect);
	AddTest("Sphere-LineSegment positive intersection", TestSphereLineSegmentIntersect);
	AddTest("Sphere-Plane positive intersection", TestSpherePlaneIntersect);
	AddTest("Sphere-Sphere positive intersection", TestSphereSphereIntersect);
	AddTest("Sphere-Triangle positive intersection", TestSphereTriangleIntersect);
	AddTest("Sphere-Capsule positive intersection", TestSphereCapsuleIntersect);
	AddTest("Sphere-Frustum positive intersection", TestSphereFrustumIntersect);
	AddTest("Sphere-Polygon positive intersection", TestSpherePolygonIntersect);
	AddTest("Sphere-Polyhedron positive intersection", TestSpherePolyhedronIntersect);

	AddTest("Frustum-Line positive intersection", TestFrustumLineIntersect);
	AddTest("Frustum-Ray positive intersection", TestFrustumRayIntersect);
	AddTest("Frustum-LineSegment positive intersection", TestFrustumLineSegmentIntersect);
	AddTest("Frustum-Plane positive intersection", TestFrustumPlaneIntersect);
	AddTest("Frustum-Triangle positive intersection", TestFrustumTriangleIntersect);
	AddTest("Frustum-Capsule positive intersection", TestFrustumCapsuleIntersect);
	AddTest("Frustum-Frustum positive intersection", TestFrustumFrustumIntersect);
	AddTest("Frustum-Polygon positive intersection", TestFrustumPolygonIntersect);
	AddTest("Frustum-Polyhedron positive intersection", TestFrustumPolyhedronIntersect);

	AddTest("Capsule-Line positive intersection", TestCapsuleLineIntersect);
	AddTest("Capsule-Ray positive intersection", TestCapsuleRayIntersect);
	AddTest("Capsule-LineSegment positive intersection", TestCapsuleLineSegmentIntersect);
	AddTest("Capsule-Plane positive intersection", TestCapsulePlaneIntersect);
	AddTest("Capsule-Triangle positive intersection", TestCapsuleTriangleIntersect);
	AddTest("Capsule-Capsule positive intersection", TestCapsuleCapsuleIntersect);
	AddTest("Capsule-Polygon positive intersection", TestCapsulePolygonIntersect);
	AddTest("Capsule-Polyhedron positive intersection", TestCapsulePolyhedronIntersect);

	AddTest("Polyhedron-Line positive intersection", TestPolyhedronLineIntersect);
	AddTest("Polyhedron-Ray positive intersection", TestPolyhedronRayIntersect);
	AddTest("Polyhedron-LineSegment positive intersection", TestPolyhedronLineSegmentIntersect);
	AddTest("Polyhedron-Plane positive intersection", TestPolyhedronPlaneIntersect);
	AddTest("Polyhedron-Triangle positive intersection", TestPolyhedronTriangleIntersect);
	AddTest("Polyhedron-Polygon positive intersection", TestPolyhedronPolygonIntersect);
	AddTest("Polyhedron-Polyhedron positive intersection", TestPolyhedronPolyhedronIntersect);

	AddTest("Polygon-Line positive intersection", TestPolygonLineIntersect);
	AddTest("Polygon-Ray positive intersection", TestPolygonRayIntersect);
	AddTest("Polygon-LineSegment positive intersection", TestPolygonLineSegmentIntersect);
	AddTest("Polygon-Plane positive intersection", TestPolygonPlaneIntersect);
	AddTest("Polygon-Triangle positive intersection", TestPolygonTriangleIntersect);
	AddTest("Polygon-Polygon positive intersection", TestPolygonPolygonIntersect);

	AddTest("Triangle-Line positive intersection", TestTriangleLineIntersect);
	AddTest("Triangle-Ray positive intersection", TestTriangleRayIntersect);
	AddTest("Triangle-LineSegment positive intersection", TestTriangleLineSegmentIntersect);
	AddTest("Triangle-Plane positive intersection", TestTrianglePlaneIntersect);
	AddTest("Triangle-Triangle positive intersection", TestTriangleTriangleIntersect);

	AddTest("Plane-Line positive intersection", TestPlaneLineIntersect);
	AddTest("Plane-Ray positive intersection", TestPlaneRayIntersect);
	AddTest("Plane-LineSegment positive intersection", TestPlaneLineSegmentIntersect);
	AddTest("Plane-Plane positive intersection", TestPlanePlaneIntersect);

	RunTests(100000);
}