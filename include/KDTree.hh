#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <memory> // for std::shared_ptr
#include <cmath> // for std::abs
#include <cstddef> // for size_t
#include <algorithm> // for std::sort
#include <cassert>

template<typename T>
class LeafNode;

template<typename T>
class InternalNode;

template<typename T>
double distance(T a, T b){
	double output = 0;
	for(int i=0; i<T::dimensions; i++){
		output += (a.get(i)-b.get(i)) * (a.get(i)-b.get(i));
	}
	return std::sqrt(output);
}

template<typename T>
class NodeBase{
public:
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
	virtual std::pair<double,LeafNode<T>*> GetClosestNode(T query) = 0;
	void SetParent(NodeBase<T>* par){parent = par;}
	friend class InternalNode<T>;

	NodeBase<T>* parent;
};

template<typename T>
class LeafNode : public NodeBase<T> {
public:
	LeafNode(T value, int repeats=1) : value(value), times_used(0), repeats(repeats) {}

	virtual int GetNumLeaves(){
		return repeats - times_used;
	}
	virtual void ReduceLeaves(){
		times_used++;
	}

	T GetValue(){
		return value;
	}

private:
	virtual std::pair<double, LeafNode<T>* > GetClosestNode(T query){
		return {distance(query,value),this};
	}

	int times_used;
	int repeats;
	T value;
};

template<typename T>
T NodeBase<T>::GetClosest(T query){
	auto res = GetClosestNode(query);
	return res.second->GetValue();
}

template<typename T>
T NodeBase<T>::PopClosest(T query){
	auto res = GetClosestNode(query);
	NodeBase<T>* node_ptr = res.second;
	while(true){
		node_ptr->ReduceLeaves();
		node_ptr = node_ptr->parent;
		if(node_ptr == nullptr){
			break;
		}
	}
	return res.second->GetValue();
}

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
	virtual std::pair<double, LeafNode<T>* > GetClosestNode(T query){
		// If one of the branches is empty, this becomes really easy.
		if(left->GetNumLeaves() == 0){
			return right->GetClosestNode(query);
		} else if (right->GetNumLeaves() == 0){
			return left->GetClosestNode(query);
		}

		// cout << "Comparing dimension " << dimension << " value " << query.get(dimension)
		// 		 << " to median value " << median << endl;

		// Check on the side that is recommended by the median heuristic.
		double diff = query.get(dimension) - median;
		auto res1 = (diff<0) ? left->GetClosestNode(query) : right->GetClosestNode(query);
		if(std::abs(diff) > res1.first){
			return res1;
		}

		// Couldn't bail out early, so check on the other side and compare.
		auto res2 = (diff<0) ? right->GetClosestNode(query) : left->GetClosestNode(query);
		return (res1.first < res2.first) ? res1 : res2;
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

	T PopClosest(T query){
		return root->PopClosest(query);
	}

	T GetClosest(T query){
		return root->GetClosest(query);
	}

private:
	std::unique_ptr<NodeBase<T> > make_node(T* arr, size_t n, int start_dim = 0){
		assert(n>0);
		if(n==1){
			return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(arr[0]));
		} else {
			// Loop over each dimension in case all values are equal in one dimension.
			for(int dim_mod = 0; dim_mod<T::dimensions; dim_mod++){
				int dimension = (start_dim + dim_mod) % T::dimensions;

				// Sort the array according to the dimension of interest.
				std::sort(arr, arr+n,
									[dimension](T a, T b){return a.get(dimension) < b.get(dimension);});

				// Search for the median starting at n/2.
				size_t median_index;
				bool found_median = false;
				for(median_index = std::max(n/2,size_t(1)); median_index<n; median_index++){
					if(arr[median_index].get(dimension) > arr[median_index-1].get(dimension)){
						found_median = true;
						break;
					}
				}

				// Search for the median counting down.
				if(!found_median){
					for(median_index=n/2; median_index>0; median_index--){
						if(arr[median_index].get(dimension) > arr[median_index-1].get(dimension)){
							found_median = true;
							break;
						}
					}
				}

				// Will be true so long as the coordinate is not equal for everything in this dimension.
				if(found_median){

					int next_dim = (dimension+1) % T::dimensions;
					double median = arr[median_index].get(dimension);
					return std::unique_ptr<InternalNode<T> >(new InternalNode<T>(
												 make_node(arr, median_index, next_dim),
												 make_node(arr+median_index,n-median_index, next_dim),
												 dimension,median));
				}
			}

			// If we got here, then everything value remaining is equal.
			return std::unique_ptr<LeafNode<T> >(new LeafNode<T>(arr[0],n));
		}
	}

	std::unique_ptr<NodeBase<T> > root;
};

#endif /* _KDTREE_H_ */
