#pragma once
#include "SimpleMath.h"
#include "Math.h"
#include "Sphere.h"
#include "Renderer_RT.h"
#include <vector>

using namespace DirectX::SimpleMath;

namespace hakner
{
	namespace Graphics
	{
		// Courtesy of Jacco Bikker
		struct BVHNode
		{
			Vector3 AABBMin {1e30f, 1e30f, 1e30f};
			Vector3 AABBMax {-1e30f, -1e30f, -1e30f};
			unsigned int firstIndex; // Relates to nodes or primitives depending on primitiveCount
			unsigned int primitiveCount;
		};

		struct AABB
		{
			Vector3 min {1e30f, 1e30f, 1e30f};
			Vector3 max {-1e30f, -1e30f, -1e30f};
			void IncludePoint( Vector3 aPoint ) { min = VectorMin( min, aPoint ), max = VectorMax( max, aPoint ); }
			float Area() 
			{ 
				Vector3 e = max - min; // box extent
				return e.x * e.y + e.y * e.z + e.z * e.x; 
			}
		};

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
			const int splitPlaneCount { 16 };
			unsigned int nodesUsed{ 0 };
			BVHNode* bvhNodes{ nullptr };
			std::vector<Sphere>& target;

			void UpdateBVHNodeBounds(unsigned int aNodeIdx);

			float FindBestSplitPlane( BVHNode& aNode, int& aAxis, float& aSplitPos );

			float CalculateNodeCost(BVHNode& aNode);

			float EvaluateSAH( BVHNode& aNode, int aAxis, float aPos );

			void SubdivideBVHNode(unsigned int aNodeIdx);

			void IntersectBVH(Ray& aRay, HitData& aData, unsigned int aNodeIdx);
		};
	}
}