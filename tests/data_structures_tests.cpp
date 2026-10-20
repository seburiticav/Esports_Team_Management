#include "data_structures.h"

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

struct PriorityEntry {
    int priority;
    int insertionOrder;
};

struct HigherPriority {
    bool operator()(const PriorityEntry& left, const PriorityEntry& right) const
    {
        if (left.priority != right.priority) return left.priority > right.priority;
        return left.insertionOrder < right.insertionOrder;
    }
};

void testDynamicArray()
{
    DynamicArray<int> values;
    for (int value = 0; value < 20; ++value) values.pushBack(value);
    require(values.size() == 20 && values[19] == 19,
            "dynamic array growth failed");
    values.removeAt(0);
    require(values.size() == 19 && values[0] == 1,
            "dynamic array removal failed");
}

void testStack()
{
    Stack<int> values;
    values.push(1);
    values.push(2);
    require(values.top() == 2 && values.pop() == 2 && values.pop() == 1,
            "stack is not LIFO");
    require(values.empty(), "stack should be empty");
}

void testQueue()
{
    Queue<int> values;
    values.enqueue(1);
    values.enqueue(2);
    require(values.front() == 1 && values.dequeue() == 1 && values.dequeue() == 2,
            "queue is not FIFO");
    values.enqueue(3);
    values.clear();
    require(values.empty() && values.size() == 0, "queue clear failed");
}

void testPriorityQueue()
{
    PriorityQueue<PriorityEntry, HigherPriority> values;
    values.push({3, 0});
    values.push({8, 1});
    values.push({8, 2});
    require(values.pop().insertionOrder == 1
                && values.pop().insertionOrder == 2
                && values.pop().priority == 3,
            "priority queue ordering or tie stability failed");
    require(values.empty(), "priority queue should be empty");
}

void testCircularQueue()
{
    CircularQueue<int> values(5);
    for (int value = 1; value <= 7; ++value) values.enqueue(value);
    require(values.size() == 5 && values.at(0) == 3 && values.at(4) == 7,
            "circular queue did not retain the newest values");
}
}

int main()
{
    try {
        testDynamicArray();
        testStack();
        testQueue();
        testPriorityQueue();
        testCircularQueue();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    std::cout << "All custom data structure tests passed.\n";
    return 0;
}
