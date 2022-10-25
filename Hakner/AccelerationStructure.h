#pragma once
#include "SimpleMath.h"
#include "Math.h"
#include "Sphere.h"
#include "Renderer.h"
#include <vector>

using namespace DirectX::SimpleMath;

namespace hakner
{
	namespace Graphics
	{

		// TODO: Convert all of this to indices for GPU portability

		// Courtesy of Jacco Bikker
		struct BVHNode
		{
			Vector3 AABBMin, AABBMax;
			unsigned int firstPrimitive;
			unsigned int primitiveCount;
			unsigned int leftNodeIdx;
		};

		inline Vector3 VectorMin(Vector3& a, Vector3& b)
		{
			return
			{
				fminf(a.x, b.x),
				fminf(a.y, b.y),
				fminf(a.z, b.z)
			};
		}

		inline Vector3 VectorMax(Vector3& a, Vector3& b)
		{
			return
			{
				fmaxf(a.x, b.x),
				fmaxf(a.y, b.y),
				fmaxf(a.z, b.z)
			};
		}

		inline bool IntersectAABB( const Ray& ray, const Vector3 bmin, const Vector3 bmax )
		{
			Vector3 o = ray.origin;
			Vector3 d = ray.direction;

			float tx1 = (bmin.x - o.x) / d.x, tx2 = (bmax.x - o.x) / d.x;
			float tmin = fminf( tx1, tx2 ), tmax = fmaxf( tx1, tx2 );
			float ty1 = (bmin.y - o.y) / d.y, ty2 = (bmax.y - o.y) / d.y;
			tmin = fmaxf( tmin, fminf( ty1, ty2 ) ), tmax = fminf( tmax, fmaxf( ty1, ty2 ) );
			float tz1 = (bmin.z - o.z) / d.z, tz2 = (bmax.z - o.z) / d.z;
			tmin = fmaxf( tmin, fminf( tz1, tz2 ) ), tmax = fminf( tmax, fmaxf( tz1, tz2 ) );
			return tmax >= tmin && tmin < ray.max && tmax > 0;
		}

		// BVH Acceleration Structure
		struct BVHAS
		{
			void IntersectBVH(Ray& ray, HitData& data);
			BVHAS(std::vector<Sphere>& aTarget);

		private:

			unsigned int nodesUsed{ 0 };
			BVHNode* bvhNodes{ nullptr };
			std::vector<Sphere>& target;

			void UpdateBVHNodeBounds(BVHNode& aNode);

			void SubdivideBVHNode(BVHNode& aNode);

			void IntersectBVH(Ray& ray, HitData& data, BVHNode& node);
		};
	}
}