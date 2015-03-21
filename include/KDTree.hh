#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <algorithm> // for std::sort
#include <cassert>
#include <cfloat>
#include <cmath> // for std::abs
#include <cstddef> // for size_t
#include <iostream>
#include <list>
#include <memory> // for std::shared_ptr
#include <tuple>
#include <vector>

template<typename T>
class LeafNode;

template<typename T>
class InternalNode;

template<typename T>
double distance2(T a, T b){
	double output = 0;
	for(int i=0; i<T::dimensions; i++){
		output += (a.get(i)-b.get(i)) * (a.get(i)-b.get(i));
	}
	return output;
}

template<typename T>
class NodeBase{
public:
	typedef std::tuple<double, LeafNode<T>*, size_t> SearchRes;

	NodeBase() : parent(nullptr) {}

	// Gets the number of leaves that are direct or indirect children.
	virtual int GetNumLeaves() = 0;
	// Pops the closest value from the tree.
	T PopClosest(T query);
	// Returns the closest value from the tree.
	T GetClosest(T query);
	// Mark one leaf as having been finished.
	virtual void ReduceLeaves() = 0;

private:
	// Returns a (distance,leafnode) pair of the closest value.
	virtual SearchRes GetClosestNode(T query) = 0;
	void SetParent(NodeBase<T>* par){parent = par;}
	friend class InternalNode<T>;

	NodeBase<T>* parent;
};

template<typename T>
class LeafNode : public NodeBase<T> {
public:
	LeafNode(std::vector<T> p_values)
		: values(std::move(p_values)), used(values.size(),false), leaves_unused(values.size()) {
		assert(leaves_unused>0);
	}

	virtual int GetNumLeaves(){
		return leaves_unused;
	}
	virtual void ReduceLeaves(){
		leaves_unused--;
	}
	void MarkAsUsed(size_t index){
		used[index] = true;
	}

	T GetValue(size_t index){
		return values[index];
	}

	T PopValue(size_t index){
		used[index] = true;
		return values[index];
	}

private:
	virtual typename NodeBase<T>::SearchRes GetClosestNode(T query){
		assert(leaves_unused > 0);

		double best_distance2 = DBL_MAX;
		size_t best_index;
		for(size_t i=0; i<values.size(); i++){
			if(!used[i]){
				double dist2 = distance2(values[i],query);
				if(dist2 < best_distance2){
					best_distance2 = dist2;
					best_index = i;
				}
			}
		}
		return std::make_tuple(best_distance2, this, best_index);
	}

	std::vector<T> values;
	std::vector<bool> used;
	int leaves_unused;
};

template<typename T>
T NodeBase<T>::GetClosest(T query){
	auto res = GetClosestNode(query);
	return std::get<1>(res)->GetValue();
}

template<typename T>
T NodeBase<T>::PopClosest(T query){
	auto res = GetClosestNode(query);
	NodeBase<T>* node_ptr = std::get<1>(res);
	while(true){
		node_ptr->ReduceLeaves();
		node_ptr = node_ptr->parent;
		if(node_ptr == nullptr){
			break;
		}
	}
	return std::get<1>(res)->PopValue(std::get<2>(res));
}

struct distance_index{
	distance_index(int index = 0, double dist2=0)
		: index(index), dist2(dist2) { }
	double dist2;
	int index;
};

template<typename T>
class InternalNode : public NodeBase<T>{
public:
	InternalNode(std::vector<std::unique_ptr<NodeBase<T> > > children, T median)
		: children(std::move(children)), median(median) {
		num_leaves = 0;
		for(auto& child : this->children){
			child->SetParent(this);
			num_leaves += child->GetNumLeaves();
		}
	}

	virtual int GetNumLeaves(){
		return num_leaves;
	}
	virtual void ReduceLeaves(){
		num_leaves--;
	}
private:
	std::array<distance_index, 1<< T::dimensions > distances;

	virtual typename NodeBase<T>::SearchRes GetClosestNode(T query){
		assert(num_leaves > 0);
		int num_children = 1 << T::dimensions;

		// Determine the minimum distance to a point in each quadrant.
		// Sum the square of the distance to each boundary that must be crossed.
		for(int i=0; i<num_children; i++){
			distances[i].dist2 = 0;
			distances[i].index = i;
			for(int j=0; j < T::dimensions; j++){
				bool point_below_median = query.get(j) < median.get(j);
				bool quadrant_below_median = (7-i) & (1<<j);
				if(point_below_median ^ quadrant_below_median){
					double dist_to_median = query.get(j) - median.get(j);
					distances[i].dist2 += dist_to_median * dist_to_median;
				}
			}
		}
		std::sort(distances.begin(), distances.end(),
				  [](const distance_index& a, const distance_index& b){
					  return a.dist2 < b.dist2;
				  });

		// Starting from the closest quadrant, check each.
		// If the quadrant could not contain anything, return early.
		typename NodeBase<T>::SearchRes output(std::numeric_limits<double>::max(),
											   NULL, 0);
		for(auto& val : distances){
			// If the best is better than anything else possible, return early.
			if(std::get<0>(output) < val.dist2){
				return output;
			}
			int leafnum = val.index;

			// Don't search an empty node.
			if(children[leafnum]->GetNumLeaves() == 0){
				continue;
			}

			// Search, and replace if better.
			auto next_res = children[leafnum]->GetClosestNode(query);
			if(std::get<0>(next_res) < std::get<0>(output)){
				output = next_res;
			}
		}
		return output;
	}

	// Leaves are in binary order, according to being above or below the median
	//  in each dimension.
	// That is, if i has binary representation k_0, k_1, ..., k_n,
	//  then i contains values that are above the median for each k_j that is 1.
	std::vector<std::unique_ptr<NodeBase<T> > > children;
	int num_leaves;
	T median;
};

template<typename T>
class KDTree{
public:
	KDTree(std::vector<T> vec){
		std::random_shuffle(vec.begin(), vec.end());
		root = make_node(vec.data(), vec.size());
	}

	T PopClosest(T query){
		return root->PopClosest(query);
	}

	T GetClosest(T query){
		return root->GetClosest(query);
	}

private:
	std::unique_ptr<NodeBase<T> > make_node(T* arr, size_t n){
		assert(n>0);
		if(n < 100){
			std::vector<T> elements = {arr,arr+n};
			assert(elements.size() > 0);
			return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(elements));
		} else {
			// Determine the median point in n dimensions.
			// Has runtime of O(k*n), where k is number of points and n is dimension
			T median;
			for(int dimension = 0; dimension<T::dimensions; dimension++){
				std::nth_element(arr, arr+n/2, arr+n,
								 [dimension](const T& a, const T& b){
									 return a.get(dimension) < b.get(dimension);
								 });
				median.set(dimension, arr[n/2].get(dimension));
			}

			// Partitions the array according to the value in each dimension.
			std::list<T*> boundaries;
			boundaries.push_back(arr);
			boundaries.push_back(arr+n);
			for(int dimension = 0; dimension<T::dimensions; dimension++){
				for(auto it = ++boundaries.begin(); it!=boundaries.end(); it++){
					auto prev = std::prev(it);
					T* median_index = std::partition(*prev, *it,
													 [dimension,&median](const T& a){
														   return a.get(dimension) < median.get(dimension);
													   });
					boundaries.insert(it, median_index);
				}
			}

			// Make each child node to be used in making the InternalNode.
			std::vector<std::unique_ptr<NodeBase<T> > > children;
			auto one_before_end = boundaries.end();
			one_before_end--;
			for(auto it = boundaries.begin(); it!=one_before_end; it++){
				auto next = std::next(it);
				children.push_back(make_node(*it, *next - *it));
			}
			return std::unique_ptr<InternalNode<T> >(new InternalNode<T>(std::move(children), median));
		}
			// // If we got here, then everything value remaining is equal.
			// std::vector<T> elements(arr,arr+n);
			// assert(elements.size() > 0);
			// return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(elements));
	}

	std::unique_ptr<NodeBase<T> > root;
};

#endif /* _KDTREE_H_ */
