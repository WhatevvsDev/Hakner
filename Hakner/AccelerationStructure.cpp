#pragma once
#include "AccelerationStructure.h"
#include "SimpleMath.h"
#include "Math.h"
#include "Sphere.h"
#include <vector>
#include <memory>

using namespace DirectX::SimpleMath;

namespace hakner
{
	namespace Graphics
	{
		BVHAS::BVHAS(std::vector<Sphere>& aTarget) : target(aTarget)
		{
			int maxNodeCount = (aTarget.size() * 2);

			// ---------- Create array big enough to hold all possible BVH Nodes ---------- 
			bvhNodes = (BVHNode*)_aligned_malloc(sizeof(BVHNode) * maxNodeCount, 32);

			if(!bvhNodes)
			{
				LogAssert("Failed to create aligned memory for BVH Nodes", bvhNodes);
				return;
			}
			nodesUsed += 2; // Is 2 for alignment

			// ---------- Create initial BVH Node ---------- 
			unsigned int startIdx = 0;
			BVHNode& startNode = bvhNodes[startIdx];
			startNode.firstIndex = 0;
			startNode.primitiveCount = aTarget.size();

			// ---------- Update AABB Bounds of initial node ---------- 
			UpdateBVHNodeBounds(startIdx);

			// ---------- Begin recursive subdividing ---------- 
			SubdivideBVHNode(startIdx);
		}

		void BVHAS::UpdateBVHNodeBounds(unsigned int aNodeIdx)
		{
			// QoL
			BVHNode& aNode = bvhNodes[aNodeIdx];

			// ---------- Set bounds to extreme values ---------- 
			aNode.AABBMin = Vector3(1e30f);
			aNode.AABBMax = Vector3(-1e30f);

			unsigned int rangeStart = aNode.firstIndex;
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

		void BVHAS::SubdivideBVHNode(unsigned int aNodeIdx)
		{
			// QoL
			BVHNode& aNode = bvhNodes[aNodeIdx];

			// ---------- Prevent nodes containing only 1 primitive ---------- 
			if (aNode.primitiveCount <= 2) 
				return;

			// ---------- Calculate largest axis ---------- 
			Vector3 extent = aNode.AABBMax - aNode.AABBMin;

			int axis = 0;
			if (extent.y > extent.x) 
				axis = 1;
			if (extent.z > extent[axis]) 
				axis = 2;

			float splitPos = aNode.AABBMin[axis] + extent[axis] * 0.5f;

			// determine split axis using SAH
			int bestAxis = -1;
			float bestPos = 0;
			float bestCost = 1e30f;

			/*
			// For each axis and each primitive
			for( int axis = 0; axis < 3; axis++ ) 
			for( unsigned int i = 0; i < aNode.primitiveCount; i++ )
			{
				Sphere& primitive = target[aNode.leftNodeIdx + i];
				float candidatePos = triangle.centroid[axis];
				float cost = EvaluateSAH( node, axis, candidatePos );
				if (cost < bestCost) 
					bestPos = candidatePos, bestAxis = axis, bestCost = cost;
			}
			int axis = bestAxis;
			float splitPos = bestPos;
			*/

			// in-place partition
			int i = aNode.firstIndex;
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
			int leftCount = i - aNode.firstIndex;
			if (leftCount == 0 || leftCount == aNode.primitiveCount) 
				return;

			// create child nodes

			int leftIdx = nodesUsed;
			BVHNode& left  = bvhNodes[leftIdx];
			BVHNode& right = bvhNodes[leftIdx + 1];

			left.firstIndex = aNode.firstIndex;
			left.primitiveCount = leftCount;
			right.firstIndex = i; // couldn't this be left[firstprim] + leftCount?
			right.primitiveCount = aNode.primitiveCount - leftCount;
			nodesUsed += 2;

			aNode.firstIndex = leftIdx;
			aNode.primitiveCount = 0;
			UpdateBVHNodeBounds(leftIdx);
			UpdateBVHNodeBounds(leftIdx + 1);

			// recurse
			SubdivideBVHNode(leftIdx);
			SubdivideBVHNode(leftIdx + 1);
		}

		void BVHAS::IntersectBVH(Ray& aRay, HitData& aData)
		{
			IntersectBVH(aRay, aData, 0);
		}	

		void BVHAS::IntersectBVH(Ray& aRay, HitData& aData, unsigned int aNodeIdx)
		{
			// QoL
			BVHNode& aNode = bvhNodes[aNodeIdx];

			if (!IntersectAABB(aRay, aNode.AABBMin, aNode.AABBMax)) 
				return;

			aData.bvhIntersections++;

			if (aNode.primitiveCount != 0)
			{
				for (unsigned int i = 0; i < aNode.primitiveCount; i++ )
					target[aNode.firstIndex + i].Intersect( aRay, aData);
			}
			else
			{
				IntersectBVH(aRay, aData, aNode.firstIndex);
				IntersectBVH(aRay, aData, aNode.firstIndex + 1);
			}
		}
	}
}