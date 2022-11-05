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

		float BVHAS::EvaluateSAH(BVHNode& aNode, int aAxis, float aPos)
		{
			// determine triangle counts and bounds for this split candidate
			AABB leftBox, rightBox;
			int leftCount = 0, rightCount = 0;
			for( unsigned int i = 0; i < aNode.primitiveCount; i++ )
			{
				Sphere& primitive = target[aNode.firstIndex + i];
				if (primitive.position[aAxis] < aPos)
				{
					leftCount++;
					leftBox.IncludePoint( primitive.position );
				}
				else
				{
					rightCount++;
					rightBox.IncludePoint( primitive.position );
				}
			}
			float cost = leftCount * leftBox.Area() + rightCount * rightBox.Area();
			return cost > 0 ? cost : 1e30f;		
		}

		void BVHAS::SubdivideBVHNode(unsigned int aNodeIdx)
		{
			// QoL
			BVHNode& aNode = bvhNodes[aNodeIdx];


			// ---------- Calculate SAH ---------- 
			int splitAxis;
			float splitPos;
			float splitCost = FindBestSplitPlane( aNode, splitAxis, splitPos );
			
			// ---------- Prevent splitting unnecessarily ----------
			if (splitCost >= CalculateNodeCost(aNode)) 
				return;

			// in-place partition
			int i = aNode.firstIndex;
			int j = i + aNode.primitiveCount - 1;

			while (i <= j)
			{
				if (target[i].position[splitAxis] < splitPos)
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

		float BVHAS::FindBestSplitPlane(BVHNode& aNode, int& aAxis, float& aSplitPos)
		{	
			float bestCost = 1e30f;

			// ---------- Check all axis ---------- 
			for (int a = 0; a < 3; a++) 
			{
				float boundsMin = aNode.AABBMin[a];
				float boundsMax = aNode.AABBMax[a];

				if (boundsMin == boundsMax) 
					continue;

				// ---------- Check intervals along axis ---------- 
				float scale = (boundsMax - boundsMin) / (float)splitPlaneCount;
				for (unsigned int i = 1; i < splitPlaneCount; i++)
				{
					float candidatePos = boundsMin + i * scale;
					float cost = EvaluateSAH( aNode, a, candidatePos );

					if (cost >= bestCost) 
						continue;

					aSplitPos = candidatePos;
					aAxis = a; 
					bestCost = cost;
				}
			}
			return bestCost;
		}
		
		float BVHAS::CalculateNodeCost(BVHNode& aNode)
		{
			Vector3 e = aNode.AABBMax - aNode.AABBMin; // extent of parent
			float parentArea = e.x * e.y + e.y * e.z + e.z * e.x;
			return aNode.primitiveCount * parentArea;
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
				srand(aNodeIdx);
				aData.bvhColor.r = 128 + (rand() % 127);
				aData.bvhColor.g = 128 + (rand() % 127);
				aData.bvhColor.b = 128 + (rand() % 127);

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