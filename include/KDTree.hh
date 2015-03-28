#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <algorithm> // for std::sort
#include <cassert>
#include <cfloat>
#include <cmath> // for std::abs
#include <cstddef> // for size_t
#include <iostream>
#include <memory> // for std::shared_ptr
#include <tuple>
#include <vector>

template<typename T>
double distance2(T a, T b){
	double output = 0;
	for(int i=0; i<T::dimensions; i++){
		output += (a.get(i)-b.get(i)) * (a.get(i)-b.get(i));
	}
	return output;
}

struct Node{
	size_t i_start, n;
	char dimension;
	double median;
	size_t unused;
	size_t left, right;
};

struct SearchRes{
	SearchRes(size_t index, double dist2)
		: index(index), dist2(dist2) { }
	size_t index;
	double dist2;
};

template<typename T>
class KDTree{
public:
	KDTree(std::vector<T> vec)
		: search_points(std::move(vec)) {
		nodes.reserve(2*search_points.size());
		make_node(0, search_points.size());
	}

	T PopClosest(T query, double epsilon = 0){
		auto res = GetClosestIndex(query, epsilon);

		// Mark the result as having been used.
		size_t node_index = 0;
		nodes[0].unused--;
		while(nodes[node_index].n != 1 &&
			  nodes[node_index].dimension != -1) {
			node_index = (res.index < nodes[nodes[node_index].right].i_start)
				? nodes[node_index].left
				: nodes[node_index].right;

			nodes[node_index].unused--;
		}

		return search_points[res.index];
	}

	T GetClosest(T query, double epsilon = 0){
		auto res = GetClosestIndex(query, epsilon);
		return search_points[res.index];
	}

private:
	SearchRes GetClosestIndex(T query, double epsilon, int node_index = 0){
		Node& node = nodes[node_index];

		// End condition, I am at a leaf node.
		if(node.n == 1 ||
		   node.dimension == -1){
			int result_index = node.i_start;
			double dist2 = distance2(search_points[result_index], query);
			return {result_index, dist2};
		}

		// If one of the branches is empty, use the other one.
		if(nodes[node.left].unused == 0){
			return GetClosestIndex(query, epsilon, node.right);
		} else if(nodes[node.right].unused == 0){
			return GetClosestIndex(query, epsilon, node.left);
		}

		// Check on the side that is recommended by the median.
		double diff = query.get(node.dimension) - node.median;
		auto res1 = (diff<0) ?
			GetClosestIndex(query, epsilon, node.left) :
			GetClosestIndex(query, epsilon, node.right);

		double allowed_diff = diff*(1+epsilon);
		if(res1.dist2 < allowed_diff * allowed_diff){
			return res1;
		}

		auto res2 = (diff<0) ?
			GetClosestIndex(query, epsilon, node.right) :
			GetClosestIndex(query, epsilon, node.left);

		return (res1.dist2 < res2.dist2) ? res1 : res2;
	}

	size_t make_node(size_t i_start, size_t n, int start_dim = 0){
		assert(n>0);
		assert(i_start >= 0);
		assert(i_start + n <= search_points.size());


		size_t node_index = nodes.size();
		nodes.emplace_back();
		Node& node = nodes[node_index];

		node.i_start = i_start;
		node.n = n;
		node.unused = n;

		if(n == 1){
			return node_index;
		} else {
			// Loop over each dimension in case all values are equal in one dimension.
			for(int dim_mod = 0; dim_mod<T::dimensions; dim_mod++){
				int dimension = (start_dim + dim_mod) % T::dimensions;

				// Find the median value.
				auto region_start = search_points.begin() + i_start;
				std::nth_element(region_start, region_start + n/2, region_start + n,
								 [dimension](T a, T b){return a.get(dimension) < b.get(dimension);});
				double median_value = (region_start + n/2)->get(dimension);

				// Find the median index
				auto median = std::partition(region_start, region_start + n,
											 [dimension, median_value](T a){
												 return a.get(dimension) < median_value;
											 });
				size_t median_index = median - region_start;

				// Will be true so long as the coordinate is not equal for everything in this dimension.
				if(median_index != 0 && median_index != n){
					node.dimension = dimension;
					node.median = (region_start + median_index)->get(dimension);

					int next_dim = (dimension+1) % T::dimensions;

					int left_index = make_node(i_start, median_index, next_dim);
					node.left = left_index;
					int right_index = make_node(i_start + median_index, n - median_index, next_dim);
					node.right = right_index;
					return node_index;
				}
			}

			// If we got here, then everything remaining is equal.
			node.dimension = -1;
			return node_index;
		}
	}

	std::vector<Node> nodes;
	std::vector<T> search_points;
};

#endif /* _KDTREE_H_ */
