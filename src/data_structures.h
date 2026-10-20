#ifndef ESPORTS_DATA_STRUCTURES_H
#define ESPORTS_DATA_STRUCTURES_H

#include <stdexcept>

template <typename T>
class DynamicArray {
public:
    explicit DynamicArray(int capacity = 8)
        : capacity_(checkedCapacity(capacity)), data_(new T[capacity_])
    {
    }

    DynamicArray(const DynamicArray& other)
        : capacity_(other.capacity_), size_(other.size_), data_(new T[capacity_])
    {
        for (int index = 0; index < size_; ++index) {
            data_[index] = other.data_[index];
        }
    }

    DynamicArray& operator=(const DynamicArray& other)
    {
        if (this == &other) return *this;

        T* replacement = new T[other.capacity_];
        for (int index = 0; index < other.size_; ++index) {
            replacement[index] = other.data_[index];
        }
        delete[] data_;
        data_ = replacement;
        capacity_ = other.capacity_;
        size_ = other.size_;
        return *this;
    }

    ~DynamicArray()
    {
        delete[] data_;
    }

    void pushBack(const T& value)
    {
        if (size_ == capacity_) grow();
        data_[size_++] = value;
    }

    void removeAt(int index)
    {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("dynamic array index out of range");
        }
        for (int position = index; position + 1 < size_; ++position) {
            data_[position] = data_[position + 1];
        }
        --size_;
    }

    void clear() { size_ = 0; }

    T& operator[](int index)
    {
        checkIndex(index);
        return data_[index];
    }

    const T& operator[](int index) const
    {
        checkIndex(index);
        return data_[index];
    }

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }

private:
    static int checkedCapacity(int capacity)
    {
        if (capacity <= 0) {
            throw std::invalid_argument("dynamic array capacity must be positive");
        }
        return capacity;
    }

    void checkIndex(int index) const
    {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("dynamic array index out of range");
        }
    }

    void grow()
    {
        const int newCapacity = capacity_ * 2;
        T* replacement = new T[newCapacity];
        for (int index = 0; index < size_; ++index) {
            replacement[index] = data_[index];
        }
        delete[] data_;
        data_ = replacement;
        capacity_ = newCapacity;
    }

    int capacity_ = 0;
    int size_ = 0;
    T* data_ = nullptr;
};

template <typename T>
class Stack {
public:
    void push(const T& value) { items_.pushBack(value); }

    T pop()
    {
        if (empty()) throw std::runtime_error("stack is empty");
        T value = top();
        items_.removeAt(items_.size() - 1);
        return value;
    }

    T& top()
    {
        if (empty()) throw std::runtime_error("stack is empty");
        return items_[items_.size() - 1];
    }

    const T& top() const
    {
        if (empty()) throw std::runtime_error("stack is empty");
        return items_[items_.size() - 1];
    }

    bool empty() const { return items_.empty(); }
    int size() const { return items_.size(); }
    void clear() { items_.clear(); }

private:
    DynamicArray<T> items_;
};

template <typename T>
class Queue {
    struct Node {
        explicit Node(const T& value) : value(value) {}
        T value;
        Node* next = nullptr;
    };

public:
    Queue() = default;
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    ~Queue()
    {
        clear();
    }

    void enqueue(const T& value)
    {
        Node* node = new Node(value);
        if (tail_) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
    }

    T dequeue()
    {
        if (empty()) throw std::runtime_error("queue is empty");
        Node* node = head_;
        T value = node->value;
        head_ = node->next;
        if (!head_) tail_ = nullptr;
        delete node;
        --size_;
        return value;
    }

    const T& front() const
    {
        if (empty()) throw std::runtime_error("queue is empty");
        return head_->value;
    }

    void clear()
    {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    bool empty() const { return head_ == nullptr; }
    int size() const { return size_; }

private:
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    int size_ = 0;
};

template <typename T>
class CircularQueue {
public:
    explicit CircularQueue(int capacity = 5)
        : capacity_(checkedCapacity(capacity)), data_(new T[capacity_])
    {
    }

    CircularQueue(const CircularQueue& other)
        : capacity_(other.capacity_), head_(other.head_), size_(other.size_),
          data_(new T[capacity_])
    {
        for (int index = 0; index < capacity_; ++index) {
            data_[index] = other.data_[index];
        }
    }

    CircularQueue& operator=(const CircularQueue& other)
    {
        if (this == &other) return *this;

        T* replacement = new T[other.capacity_];
        for (int index = 0; index < other.capacity_; ++index) {
            replacement[index] = other.data_[index];
        }
        delete[] data_;
        data_ = replacement;
        capacity_ = other.capacity_;
        head_ = other.head_;
        size_ = other.size_;
        return *this;
    }

    ~CircularQueue()
    {
        delete[] data_;
    }

    void enqueue(const T& value)
    {
        if (full()) {
            data_[head_] = value;
            head_ = (head_ + 1) % capacity_;
            return;
        }
        data_[(head_ + size_) % capacity_] = value;
        ++size_;
    }

    const T& at(int index) const
    {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("circular queue index out of range");
        }
        return data_[(head_ + index) % capacity_];
    }

    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == capacity_; }
    int size() const { return size_; }
    int capacity() const { return capacity_; }

private:
    static int checkedCapacity(int capacity)
    {
        if (capacity <= 0) {
            throw std::invalid_argument("circular queue capacity must be positive");
        }
        return capacity;
    }

    int capacity_ = 0;
    int head_ = 0;
    int size_ = 0;
    T* data_ = nullptr;
};

template <typename T, typename Compare>
class PriorityQueue {
public:
    explicit PriorityQueue(Compare compare = Compare())
        : compare_(compare)
    {
    }

    void push(const T& value)
    {
        heap_.pushBack(value);
        siftUp(heap_.size() - 1);
    }

    T pop()
    {
        if (empty()) throw std::runtime_error("priority queue is empty");
        T value = heap_[0];
        heap_[0] = heap_[heap_.size() - 1];
        heap_.removeAt(heap_.size() - 1);
        if (!empty()) siftDown(0);
        return value;
    }

    bool empty() const { return heap_.empty(); }
    int size() const { return heap_.size(); }

private:
    void siftUp(int index)
    {
        while (index > 0) {
            const int parent = (index - 1) / 2;
            if (!compare_(heap_[index], heap_[parent])) break;
            T value = heap_[index];
            heap_[index] = heap_[parent];
            heap_[parent] = value;
            index = parent;
        }
    }

    void siftDown(int index)
    {
        while (true) {
            const int left = 2 * index + 1;
            const int right = left + 1;
            int highest = index;
            if (left < heap_.size() && compare_(heap_[left], heap_[highest])) {
                highest = left;
            }
            if (right < heap_.size() && compare_(heap_[right], heap_[highest])) {
                highest = right;
            }
            if (highest == index) break;
            T value = heap_[index];
            heap_[index] = heap_[highest];
            heap_[highest] = value;
            index = highest;
        }
    }

    DynamicArray<T> heap_;
    Compare compare_;
};

#endif
