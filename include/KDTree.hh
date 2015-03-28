#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <algorithm> // for std::sort
#include <cassert>
#include <cfloat>
#include <cmath> // for std::abs
#include <cstddef> // for size_t
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
	T PopClosest(T query, double epsilon){
		auto res = GetClosestNode(query, epsilon);
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

	// Returns the closest value from the tree.
	T GetClosest(T query, double epsilon){
		auto res = GetClosestNode(query, epsilon);
		return std::get<1>(res)->GetValue();
	}
	// Mark one leaf as having been finished.
	virtual void ReduceLeaves() = 0;

private:
	// Returns a (distance,leafnode) pair of the closest value.
	virtual SearchRes GetClosestNode(T query, double epsilon) = 0;
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
	virtual typename NodeBase<T>::SearchRes GetClosestNode(T query, double epsilon){
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
class InternalNode : public NodeBase<T>{
public:
	InternalNode(std::unique_ptr<NodeBase<T> > p_left, std::unique_ptr<NodeBase<T> > p_right,
							 int dimension, double median)
		: left(std::move(p_left)), right(std::move(p_right)), dimension(dimension), median(median) {
		num_leaves = left->GetNumLeaves() + right->GetNumLeaves();
		left->SetParent(this);
		right->SetParent(this);
	}

	virtual int GetNumLeaves(){
		return num_leaves;
	}
	virtual void ReduceLeaves(){
		num_leaves--;
	}
private:
	virtual typename NodeBase<T>::SearchRes GetClosestNode(T query, double epsilon){
		assert(num_leaves > 0);

		// If one of the branches is empty, this becomes really easy.
		if(left->GetNumLeaves() == 0){
			return right->GetClosestNode(query, epsilon);
		} else if (right->GetNumLeaves() == 0){
			return left->GetClosestNode(query, epsilon);
		}

		// Check on the side that is recommended by the median heuristic.
		double diff = query.get(dimension) - median;
		auto res1 = (diff<0) ? left->GetClosestNode(query, epsilon) : right->GetClosestNode(query, epsilon);
		if(diff*diff*(1+epsilon)*(1+epsilon) > std::get<0>(res1) ){
			return res1;
		}

		// Couldn't bail out early, so check on the other side and compare.
		auto res2 = (diff<0) ? right->GetClosestNode(query, epsilon) : left->GetClosestNode(query, epsilon);
		return (std::get<0>(res1) < std::get<0>(res2)) ? res1 : res2;
	}

	std::unique_ptr<NodeBase<T> > left;
	std::unique_ptr<NodeBase<T> > right;
	int num_leaves;
	int dimension;
	double median;
};

template<typename T>
class KDTree{
public:
	KDTree(std::vector<T> vec){
		root = make_node(vec.data(), vec.size());
	}

	T PopClosest(T query, double epsilon = 0){
		return root->PopClosest(query, epsilon);
	}

	T GetClosest(T query, double epsilon = 0){
		return root->GetClosest(query, epsilon);
	}

private:
	std::unique_ptr<NodeBase<T> > make_node(T* arr, size_t n, int start_dim = 0){
		assert(n>0);
		if(n < 50){
			std::vector<T> elements = {arr,arr+n};
			assert(elements.size() > 0);
			return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(elements));
		} else {
			// Loop over each dimension in case all values are equal in one dimension.
			for(int dim_mod = 0; dim_mod<T::dimensions; dim_mod++){
				int dimension = (start_dim + dim_mod) % T::dimensions;

				// Find the median value.
				std::nth_element(arr, arr+n/2, arr+n,
								 [dimension](T a, T b){return a.get(dimension) < b.get(dimension);});
				double median_value = arr[n/2].get(dimension);

				// Find the median index
				T* median = std::partition(arr, arr+n,
										   [dimension, median_value](T a){
											   return a.get(dimension) < median_value;
										   });
				size_t median_index = median - arr;

				// Will be true so long as the coordinate is not equal for everything in this dimension.
				if(median_index != 0 && median_index != n){
					int next_dim = (dimension+1) % T::dimensions;
					double median = arr[median_index].get(dimension);
					return std::unique_ptr<InternalNode<T> >(new InternalNode<T>(
												 make_node(arr, median_index, next_dim),
												 make_node(arr+median_index,n-median_index, next_dim),
												 dimension,median));
				}
			}

			// If we got here, then everything value remaining is equal.
			std::vector<T> elements(arr,arr+n);
			assert(elements.size() > 0);
			return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(elements));
		}
	}

	std::unique_ptr<NodeBase<T> > root;
};

#endif /* _KDTREE_H_ */
