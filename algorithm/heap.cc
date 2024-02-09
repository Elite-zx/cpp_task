#include <bits/stdc++.h>

#include <vector>
using namespace std;

class MaxHeap {
 public:
  // 构造函数：使用给定的数组项初始化最大堆
  MaxHeap(vector<int>& items)
      : cur_size(items.size()),  // 设置当前大小为数组的大小
        _heap(items.size() +
              10) {  // 分配比原数组大一些的空间，以便有扩展的空间
    // 将所有项复制到_heap中，从索引1开始（为了简化父子节点的计算）
    for (int i = 0; i < items.size(); i++) _heap[i + 1] = items[i];
    // 从最后一个非叶子节点开始向下调整，构建最大堆
    for (int i = cur_size / 2; i > 0; i--) heapify(i);
  }

  // heapify方法：确保从hole_pos开始的子堆满足最大堆的性质
  void heapify(int hole_pos) {
    auto tmp = std::move(_heap[hole_pos]);  // 暂存当前节点
    int child_pos;
    // 重复向下调整，直到没有子节点或已经满足最大堆的性质
    for (; hole_pos * 2 <= cur_size; hole_pos = child_pos) {
      child_pos = hole_pos * 2;  // 左子节点位置
      // 如果有右子节点，且右子节点大于左子节点，使用右子节点
      if (child_pos < cur_size && _heap[child_pos] < _heap[child_pos + 1])
        child_pos++;
      // 如果子节点大于当前节点，将子节点移动到当前节点
      if (_heap[child_pos] > tmp) {
        _heap[hole_pos] = std::move(_heap[child_pos]);
      } else {
        break;  // 已满足最大堆性质，终止调整
      }
    }
    // 将暂存的值放到最终位置
    _heap[hole_pos] = std::move(tmp);
  }

  // push方法：向堆中插入一个新的值
  void push(int value) {
    // 如果当前大小等于数组容量，扩大数组
    if (cur_size == _heap.size() - 1) _heap.resize(cur_size * 2);
    // 在数组末尾添加新元素，并向上调整以保持最大堆性质
    int hole_pos = ++cur_size;
    for (; hole_pos > 1 && value > _heap[hole_pos / 2]; hole_pos /= 2) {
      _heap[hole_pos] = std::move(_heap[hole_pos / 2]);  // 父节点下移
    }
    _heap[hole_pos] = std::move(value);  // 插入新值
  }

  // pop方法：从堆中删除最大值（堆顶元素）
  void pop() {
    // 将最后一个元素移动到堆顶，然后从堆顶开始向下调整
    _heap[1] = std::move(_heap[cur_size--]);
    heapify(1);  // 重新构建最大堆
  }

  // top方法：获取堆顶元素（最大值）
  int top() {
    if (cur_size > 0) return _heap[1];     // 返回堆顶元素
    throw runtime_error("Heap is empty");  // 如果堆为空，抛出异常
  }

 private:
  size_t cur_size;    // 当前堆中元素的数量
  vector<int> _heap;  // 存储堆元素的数组
};

class MinHeap {
 public:
  MinHeap(vector<int>& items)
      : cur_size(items.size()), _heap(items.size() + 10) {
    for (int i = 0; i < items.size(); i++) _heap[i + 1] = items[i];
    for (int i = cur_size / 2; i > 0; i--) heapify(i);
  }

  void heapify(int hole_pos) {
    auto tmp = std::move(_heap[hole_pos]);
    int child_pos{0};
    for (; hole_pos * 2 <= cur_size; hole_pos = child_pos) {
      child_pos = hole_pos * 2;
      if (child_pos < cur_size && _heap[child_pos] > _heap[child_pos + 1])
        child_pos++;
      if (_heap[child_pos] < tmp) {
        _heap[hole_pos] = std::move(_heap[child_pos]);
      } else {
        break;
      }
    }
    _heap[hole_pos] = std::move(tmp);
  }

  void push(int value) {
    if (cur_size == _heap.size() - 1) _heap.resize(cur_size * 2);
    int hole_pos = ++cur_size;
    for (; hole_pos > 1 && value < _heap[hole_pos / 2]; hole_pos /= 2) {
      _heap[hole_pos] = std::move(_heap[hole_pos / 2]);
    }
    _heap[hole_pos] = std::move(value);
  }

  void pop() {
    _heap[1] = std::move(_heap[cur_size--]);
    heapify(1);
  }

  int top() {
    if (cur_size > 0) return _heap[1];
    throw runtime_error("Heap is empty");
  }

 private:
  size_t cur_size;
  vector<int> _heap;
};

/* Top-K  */
class Solution {
 public:
  int findKthLargest(vector<int>& nums, int k) {
    vector<int> nums_within_k(nums.begin(), nums.begin() + k);
    MinHeap min_heap(nums_within_k);

    for (int i = k; i < nums.size(); i++) {
      if (nums[i] > min_heap.top()) {
        min_heap.pop();
        min_heap.push(nums[i]);
      }
    }
    return min_heap.top();
  }
};

void heapify(vector<int>& nums, int n, int i) {
  auto tmp = std::move(nums[i]);
  int child_pos{0};
  for (; (i * 2 + 1) < n; i = child_pos) {
    child_pos = i * 2 + 1;
    if (child_pos < n - 1 && nums[child_pos] < nums[child_pos + 1]) child_pos++;
    if (nums[child_pos] > tmp) {
      nums[i] = std::move(nums[child_pos]);
    } else {
      break;
    }
  }
  nums[i] = std::move(tmp);
}

vector<int> heap_sort(vector<int>& nums) {
  for (int i = (nums.size() - 1) / 2; i >= 0; i--)
    heapify(nums, nums.size(), i);

  for (int i = nums.size() - 1; i > 0; i--) {
    swap(nums[i], nums[0]);
    heapify(nums, i, 0);
  }
  return nums;
}
