#pragma once
#include "AccelerationStructure.h"
#include "SimpleMath.h"
#include "Math.h"
#include "Sphere.h"
#include <vector>

using namespace DirectX::SimpleMath;

namespace hakner
{
	namespace Graphics
	{
		BVHAS::BVHAS(std::vector<Sphere>& aTarget) : target(aTarget)
		{
			int maxNodeCount = (aTarget.size() * 2) - 1;

			bvhNodes = new BVHNode[maxNodeCount]; // TODO: align this
			nodesUsed += 2; // For (future) alignment

			// ---------- Create array big enough to hold all possible BVH Nodes ---------- 

			// ---------- Create initial BVH Node ---------- 
			BVHNode& start = bvhNodes[0];
			start.firstPrimitive = 0;
			start.primitiveCount = aTarget.size();

			// ---------- Update AABB Bounds of initial node ---------- 
			UpdateBVHNodeBounds(start);

			// ---------- Begin recursive subdividing ---------- 
			SubdivideBVHNode(start);
		}

		void BVHAS::UpdateBVHNodeBounds(BVHNode& aNode)
		{
			// ---------- Set bounds to extreme values ---------- 
			aNode.AABBMin = Vector3(1e30f);
			aNode.AABBMax = Vector3(-1e30f);

			unsigned int rangeStart = aNode.firstPrimitive;
			for (unsigned int i = 0; i < aNode.primitiveCount; i++)
			{
				unsigned int finalIndex = rangeStart + i;

				Sphere& sphere = target[finalIndex];
				Vector3 sphereMin = sphere.position + Vector3(-sphere.GetRadius());
				Vector3 sphereMax = sphere.position + Vector3(sphere.GetRadius());

				aNode.AABBMin = VectorMin(aNode.AABBMin, sphereMin);
				aNode.AABBMax = VectorMax(aNode.AABBMax, sphereMax);
			}
		}

		void BVHAS::SubdivideBVHNode(BVHNode& aNode)
		{
						// ---------- Prevent nodes containing only 1 primitive ---------- 
			if (aNode.primitiveCount <= 2) return;

			// ---------- Calculate largest axis ---------- 
			Vector3 extent = aNode.AABBMax - aNode.AABBMin;

			int axis = 0;
			if (extent.y > extent.x) 
				axis = 1;
			if (extent.z > extent[axis]) 
				axis = 2;

			float splitPos = aNode.AABBMin[axis] + extent[axis] * 0.5f;

			// in-place partition
			int i = aNode.firstPrimitive;
			int j = i + aNode.primitiveCount - 1;

			while (i <= j)
			{
				if (target[i].position[axis] < splitPos)
				{
					i++;
				}
				else
				{
					std::swap(target[i], target[j]);
					j--;
				}
			}

			// abort split if one of the sides is empty
			int leftCount = i - aNode.firstPrimitive;
			if (leftCount == 0 || leftCount == aNode.primitiveCount) 
				return;

			// create child nodes
			aNode.leftNodeIdx = nodesUsed;

			BVHNode& left  = bvhNodes[nodesUsed + 0];
			BVHNode& right = bvhNodes[nodesUsed + 1];

			left.firstPrimitive = aNode.firstPrimitive;
			left.primitiveCount = leftCount;
			right.firstPrimitive = i; // couldn't this be left[firstprim] + leftCount?
			right.primitiveCount = aNode.primitiveCount - leftCount;
			nodesUsed += 2;

			aNode.primitiveCount = 0;
			UpdateBVHNodeBounds(left);
			UpdateBVHNodeBounds(right);

			// recurse
			SubdivideBVHNode(left);
			SubdivideBVHNode(right);
		}

		void BVHAS::IntersectBVH(Ray& ray, HitData& data)
		{
			IntersectBVH(ray, data, bvhNodes[0]);
		}	

		void BVHAS::IntersectBVH(Ray& ray, HitData& data, BVHNode& node)
		{
			if (!IntersectAABB( ray, node.AABBMin, node.AABBMax )) 
				return;

			data.bvhIntersections++;

			if (node.primitiveCount != 0)
			{
				for (unsigned int i = 0; i < node.primitiveCount; i++ )
					target[node.firstPrimitive + i].Intersect( ray, data);
			}
			else
			{
				IntersectBVH( ray, data, bvhNodes[node.leftNodeIdx]);
				IntersectBVH( ray, data, bvhNodes[ node.leftNodeIdx + 1]);
			}
		}
	}
}