#pragma once                                            
struct JumpQueue {
    float* positions;
    int front;
    int rear;
    int size;
    int capacity;

    JumpQueue(int initialCapacity = 100) {
        positions = new float[initialCapacity];
        capacity = initialCapacity;
        front = 0;
        rear = 0;
        size = 0;
    }

    ~JumpQueue() {
        delete[] positions;
    }

    bool isEmpty() const { return size == 0; }

    void enqueue(float xPos) {
        if (size == capacity) {
            int newCapacity = capacity * 2;
            float* newArray = new float[newCapacity];
            for (int i = 0; i < size; ++i) {
                newArray[i] = positions[(front + i) % capacity];
            }
            delete[] positions;
            positions = newArray;
            front = 0;
            rear = size;
            capacity = newCapacity;
        }
        positions[rear] = xPos;
        rear = (rear + 1) % capacity;
        size++;
    }

    float dequeue() {
        if (isEmpty()) return 0.0f;
        float xPos = positions[front];
        front = (front + 1) % capacity;
        size--;
        return xPos;
    }

    float peek() const {
        if (isEmpty()) return 0.0f;
        return positions[front];
    }
};