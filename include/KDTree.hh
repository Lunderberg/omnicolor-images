#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <algorithm> // for std::sort
#include <cassert>
#include <cfloat> // for DBL_MAX
#include <cmath> // for std::abs
#include <cstddef> // for size_t
#include <memory> // for std::shared_ptr
#include <vector>

#include <iostream>

struct PerformanceStats {
  unsigned int nodes_checked;
  unsigned int leaf_nodes_checked;
  unsigned int points_checked;

  PerformanceStats() :
    nodes_checked(0), leaf_nodes_checked(0), points_checked(0)
    { }
};

template<typename T>
class LeafNode;

template<typename T>
class InternalNode;

template<typename T>
double distance2(const T& a, const T& b){
  double output = 0;
  for(int i=0; i<T::dimensions; i++){
    output += (a.get(i)-b.get(i)) * (a.get(i)-b.get(i));
  }
  return output;
}

template<typename T>
struct KDTree_Result {
  T res;
  PerformanceStats stats;
};

template<typename T>
class NodeBase{
public:
  struct SearchRes{
    SearchRes(double dist2, LeafNode<T>* leaf, size_t index)
      : dist2(dist2), leaf(leaf), index(index) { }
    double dist2;
    LeafNode<T>* leaf;
    size_t index;
  };

  NodeBase() : parent(nullptr) {}

  virtual ~NodeBase() {}

  // Gets the number of leaves that are direct or indirect children.
  virtual int GetNumLeaves() = 0;

  // Pops the closest value from the tree.
  KDTree_Result<T> PopClosest(T query, double epsilon){
    KDTree_Result<T> output;
    auto res = GetClosestNode(query, epsilon, output.stats);
    NodeBase<T>* node_ptr = res.leaf;
    while(true){
      node_ptr->ReduceLeaves();
      node_ptr = node_ptr->parent;
      if(node_ptr == nullptr){
        break;
      }
    }
    output.res = res.leaf->PopValue(res.index);
    return output;
  }

  // Returns the closest value from the tree.
  KDTree_Result<T> GetClosest(T query, double epsilon){
    KDTree_Result<T> output;
    auto res = GetClosestNode(query, epsilon, output.stats);
    output.res = res.leaf->GetValue(res.index);
    return output;
  }
  // Mark one leaf as having been finished.
  virtual void ReduceLeaves() = 0;

private:
  // Returns a (distance,leafnode) pair of the closest value.
  virtual SearchRes GetClosestNode(T query, double epsilon,
                                   PerformanceStats& stats) = 0;
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
  virtual typename NodeBase<T>::SearchRes GetClosestNode(T query, double /* epsilon */, PerformanceStats& stats){
    assert(leaves_unused > 0);
    assert(values.size() == used.size());

    stats.nodes_checked += 1;
    stats.leaf_nodes_checked += 1;
    stats.points_checked += leaves_unused;

    double best_distance2 = DBL_MAX;
    size_t best_index = 0;
    for(size_t i=0; i<values.size(); i++){
      if(!used[i]){
        double dist2 = distance2(values[i],query);
        if(dist2 < best_distance2){
          best_distance2 = dist2;
          best_index = i;
        }
      }
    }
    return {best_distance2, this, best_index};
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
  virtual typename NodeBase<T>::SearchRes GetClosestNode(T query, double epsilon, PerformanceStats& stats){
    assert(num_leaves > 0);

    stats.nodes_checked += 1;

    // If one of the branches is empty, this becomes really easy.
    if(left->GetNumLeaves() == 0){
      return right->GetClosestNode(query, epsilon, stats);
    } else if (right->GetNumLeaves() == 0){
      return left->GetClosestNode(query, epsilon, stats);
    }

    // Check on the side that is recommended by the median heuristic.
    double diff = query.get(dimension) - median;
    auto res1 = (diff<0) ? left->GetClosestNode(query, epsilon, stats) : right->GetClosestNode(query, epsilon, stats);
    double allowed_diff = diff*(1+epsilon);
    if(allowed_diff * allowed_diff > res1.dist2 ){
      return res1;
    }

    // Couldn't bail out early, so check on the other side and compare.
    auto res2 = (diff<0) ? right->GetClosestNode(query, epsilon, stats) : left->GetClosestNode(query, epsilon, stats);
    return (res1.dist2 < res2.dist2) ? res1 : res2;
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

  KDTree_Result<T> PopClosest(T query, double epsilon = 0){
    return root->PopClosest(query, epsilon);
  }

  KDTree_Result<T> GetClosest(T query, double epsilon = 0){
    return root->GetClosest(query, epsilon);
  }

  int GetNumLeaves(){
    return root->GetNumLeaves();
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
