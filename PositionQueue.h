#pragma once

struct PositionQueue {
    struct Position {
        float x, y;
    };
    Position* positions;
    int front;
    int rear;
    int size;
    int capacity;

    PositionQueue(int initialCapacity = 100) {
        positions = new Position[initialCapacity];
        capacity = initialCapacity;
        front = 0;
        rear = 0;
        size = 0;
    }

    ~PositionQueue() {
        delete[] positions;
    }

    bool isEmpty() const { return size == 0; }

    void enqueue(float x, float y) {
        if (size == capacity) {
            int newCapacity = capacity * 2;
            Position* newArray = new Position[newCapacity];
            for (int i = 0; i < size; ++i) {
                newArray[i] = positions[(front + i) % capacity];
            }
            delete[] positions;
            positions = newArray;
            front = 0;
            rear = size;
            capacity = newCapacity;
        }
        positions[rear].x = x;
        positions[rear].y = y;
        rear = (rear + 1) % capacity;
        size++;
    }

    Position dequeue() {
        if (isEmpty()) {
            Position pos = { 0.0f, 0.0f };
            return pos;
        }
        Position pos = positions[front];
        front = (front + 1) % capacity;
        size--;
        return pos;
    }

    Position peek() const {
        if (isEmpty()) {
            Position pos = { 0.0f, 0.0f };
            return pos;
        }
        return positions[front];
    }
};