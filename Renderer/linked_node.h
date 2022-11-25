#pragma once
#ifndef LINKED_NODE_H
#define LINKED_NODE_H

#include <iostream>
#include <string>
#include <iterator>
#include <cstddef> 
#include <vector>
#include <unordered_set>
#include <utility>
#include <map>
#include <stdint.h>

#define uptr uintptr_t

/*
* Structure for storing the values of two pointers in as size_t.  For the sole 
* purpose of making hash code generation easier.
*/
struct ptr_pair {
    size_t a;
    size_t b;

    ptr_pair() { a = NULL; b = NULL; }
    ptr_pair(void* pa, void* pb) { a = (size_t)pa; b = (size_t)pb; }

    constexpr bool operator == (const ptr_pair& other) const {
        return ((a == other.a || a == other.b) && (b == other.a || b == other.b));
    }

};

struct ptr_pair_hash {
    std::size_t operator () (const ptr_pair& pair) const {
        auto a = static_cast<unsigned long long> (pair.a);
        auto b = static_cast<unsigned long long> (pair.b);
        return std::hash<size_t>()(pair.a)^std::hash<size_t>()(pair.b);
     }
};

template<typename T>
struct linked_node {

public:
    T data;

    //the iterator gives a pointer to a linked node as it goes through the connections
    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = linked_node<T>*;
        using pointer = linked_node<T>**;
        using reference = linked_node<T>*&;

        iterator(pointer ptr) : m_ptr(ptr) {}

        reference operator*() const { return *m_ptr; }
        pointer operator->() { return m_ptr; }

        //prefix increment
        iterator& operator++() { m_ptr++; return *this; }

        //postfix increment
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr; };

    private:
        pointer m_ptr;
    };

    //constructors
    linked_node();
    linked_node(T data);

    //operators
    bool operator == (const linked_node<T>& other);

    //iterator
    iterator begin() { return iterator(this->connections); }
    iterator end() { return iterator(this->connections + this->rank); }

    //get fields
    linked_node<T>** get_connections() { return this->connections; }
    int get_rank() { return this->rank; }

    //other
    void add_node(linked_node<T>& other);
    void remove_node(linked_node<T>& other);
    void add_node_list(const std::initializer_list<linked_node<T>*>& nodes);
    void remove_node_list(const std::initializer_list<linked_node<T>*>& nodes);

    //useful things
    int total_node_count();
    template<typename func>
    void execute_func(func f);

    //copy to a std::vector
    void copy_to(linked_node<T>*& pcopy,linked_node<T>*& pcopy_end);

private:
    template<typename func>
    void execute_func_rec(std::unordered_set<ptr_pair, ptr_pair_hash>& links_trav, func f);
    int rank = 0;

    linked_node<T>** connections = nullptr;   
};


#include "linked_node.hpp"

#endif // !LINKED_NODE_H



