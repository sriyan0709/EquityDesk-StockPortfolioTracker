#ifndef PORTFOLIO_DYNAMIC_ARRAY_H
#define PORTFOLIO_DYNAMIC_ARRAY_H

#include "portfolio/core/Exceptions.h"

namespace portfolio {
namespace templates {

template <class T>
class DynamicArray {
private:
    T* elements;
    int currentSize;
    int currentCapacity;

    void resize(int newCapacity) {
        T* newElements = new T[newCapacity];

        for (int i = 0; i < currentSize; ++i) {
            newElements[i] = elements[i];
        }

        delete[] elements;

        elements = newElements;
        currentCapacity = newCapacity;
    }

public:
    DynamicArray()
        : elements(0), currentSize(0), currentCapacity(2) {

        elements = new T[currentCapacity];
    }

    DynamicArray(const DynamicArray& other)
        : elements(0),
          currentSize(other.currentSize),
          currentCapacity(other.currentCapacity) {

        elements = new T[currentCapacity];

        for (int i = 0; i < currentSize; ++i) {
            elements[i] = other.elements[i];
        }
    }

    ~DynamicArray() {
        delete[] elements;
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* newElements = new T[other.currentCapacity];

            for (int i = 0; i < other.currentSize; ++i) {
                newElements[i] = other.elements[i];
            }

            delete[] elements;

            elements = newElements;
            currentSize = other.currentSize;
            currentCapacity = other.currentCapacity;
        }

        return *this;
    }

    void pushBack(const T& value) {
        if (currentSize == currentCapacity) {
            resize(currentCapacity * 2);
        }

        elements[currentSize] = value;
        ++currentSize;
    }

    void removeAt(int index) {
        if (index < 0 || index >= currentSize) {
            throw portfolio::PortfolioException(
                "DynamicArray index out of range."
            );
        }

        for (int i = index; i < currentSize - 1; ++i) {
            elements[i] = elements[i + 1];
        }

        --currentSize;
    }

    T& at(int index) {
        if (index < 0 || index >= currentSize) {
            throw portfolio::PortfolioException(
                "DynamicArray index out of range."
            );
        }

        return elements[index];
    }

    const T& at(int index) const {
        if (index < 0 || index >= currentSize) {
            throw portfolio::PortfolioException(
                "DynamicArray index out of range."
            );
        }

        return elements[index];
    }

    T& operator[](int index) {
        return elements[index];
    }

    const T& operator[](int index) const {
        return elements[index];
    }

    int size() const {
        return currentSize;
    }

    int capacity() const {
        return currentCapacity;
    }

    bool isEmpty() const {
        return currentSize == 0;
    }

    void clear() {
        currentSize = 0;
    }
};

}
}

#endif