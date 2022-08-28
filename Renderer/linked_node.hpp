#ifndef LINKED_NODE_HPP
#define LINKED_NODE_HPP

#include "linked_node.h"

//CONSTRUCTORS
template<typename T>
linked_node<T>::linked_node() {

}

template<typename T>
linked_node<T>::linked_node(T data) {
    this->data = data;
    this->connections = nullptr;
}

//OPERATORS
template<typename T>
bool linked_node<T>::operator == (const linked_node<T>& other) {
    if (this->data != other.data) {
        return false;
    }
    if (this->rank != other.rank) {
        return false;
    }

    for (int i = 0; i < this->rank; i++) {
        for (int j = 0; j <= other.rank; j++) {
            if (j == other.rank) {
                return false;
            }
            if (*(this->connections + i) == *(other.connections + j)) {
                break;
            }
        }
    }

    return true;

}

//OTHER

//add and remove nodes
template<typename T>
void linked_node<T>::add_node(linked_node<T>& other) {
    linked_node<T>* pother = &other;
    for (int i = 0; i < this->rank; i++) {
        if (this->connections[i] == pother) {
            return;
        }
    }
    if (this == pother) {
        return;
    }

    linked_node<T>** new_connections = new linked_node<T>*[this->rank + 1];
    linked_node<T>** new_connections_other = new linked_node<T>*[pother->rank + 1];

    for (int i = 0; i < this->rank; i++) {
        *(new_connections + i) = *(this->connections + i);
    }
    for (int i = 0; i < pother->rank; i++) {
        *(new_connections_other + i) = *(pother->connections + i);
    }

    delete[] this->connections;
    delete[] pother->connections;

    *(new_connections + this->rank) = pother;
    *(new_connections_other + pother->rank) = this;

    this->connections = new_connections;
    pother->connections = new_connections_other;

    this->rank++;
    pother->rank++;
}

template<typename T>
void linked_node<T>::remove_node(linked_node<T>& other) {
    if (this->rank == 0 || other.rank == 0) {
        return;
    }

    linked_node<T>* pother = &other;

    linked_node<T>** new_connections = new linked_node<T>*[this->rank - 1];
    linked_node<T>** new_connections_other = new linked_node<T>*[pother->rank - 1];

    bool is_removed = 0;
    for (int i = 0; i < this->rank; i++) {
        linked_node<T>* p = *(this->connections + i);
        if (!(p == pother)) {
            *(new_connections + i - is_removed) = *(this->connections + i);
        }
        else {
            is_removed = true;
        }
    }

    is_removed = 0;
    for (int i = 0; i < pother->rank; i++) {
        linked_node<T>* p = *(pother->connections + i);
        if (!(p == this)) {
            *(new_connections_other + i - is_removed) = *(pother->connections + i);
        }
        else {
            is_removed = true;
        }
    }

    //delete[] this->connections;
    //delete[] pother->connections;

    this->connections = new_connections;
    pother->connections = new_connections_other;

    this->rank--;
    pother->rank--;
}

template<typename T>
void linked_node<T>::add_node_list(const std::initializer_list<linked_node<T>*>& nodes) {
    for (linked_node<T>* node : nodes) {
        this->add_node(*node);
    }
}

template<typename T>
void linked_node<T>::remove_node_list(const std::initializer_list<linked_node<T>*>& nodes) {
    for (linked_node<T>* node : nodes) {
        this->remove_node(*node);
    }
}

/*
* "Runs through" entire structure of nodes, and for every connection, calls a user
* specified function which does something with the two nodes in that connection.
* 
* @param f - function which takes two arguments of type linked_node<T>*.  It 
*            is perfectly valid to pass a function which takes more arguments,
*            as long as those arguments have default values.
*/
template<typename T> template<typename func>
inline void linked_node<T>::execute_func(func f) {
    //links_trav keeps track of the connections which have already been run through,
    //as to avoid calling f on a connection more than once.
    std::unordered_set<ptr_pair, ptr_pair_hash> links_trav;
    this->execute_func_rec(links_trav, f);
}

/*
* Recursive part of execute_func. 
*/
template<typename T>
template<typename func>
inline void linked_node<T>::execute_func_rec(std::unordered_set<ptr_pair, ptr_pair_hash>& links_trav, func f) {
    int rank = this->rank;

    //break condition
    if (rank == 0) {
        return;
    }

    //go through connections. 
    int i = 0;
    for (linked_node<T>* other : *this) {
        ptr_pair link(this, other);

        if (links_trav.count(link) == 0) {
            links_trav.insert(link);
            f(this, other);
            other->execute_func_rec(links_trav, f);
            i++;
        }
    }
    return;
}

/*
* Runs through entire structure of nodes and returns number of unique nodes.
* @return int - number of nodes
*/
template<typename T>
inline int linked_node<T>::total_node_count() {
    std::unordered_set<uptr> node_counter;

    auto get_node_count = [&node_counter](linked_node<T>* n1, linked_node<T>* n2) {
        uptr a1 = reinterpret_cast<uptr>(n1);
        uptr a2 = reinterpret_cast<uptr>(n2);
        if (node_counter.count(a1) == 0) {
            node_counter.insert(a1);
        }
        if (node_counter.count(a2) == 0) {
            node_counter.insert(a2);
        }
    };
    this->execute_func(get_node_count);
    return node_counter.size();
}

/*
* Copies entire linked node strucuture to a linked_node<T> pointer.  The node object
* from which "copy_to" is called is copied to pcopy, and each subsequent node in the
* structure is then placed into the adjacent spot in memory, with the last node being 
* copied into pcopy_end.  The order of the nodes in memory will depend on the path 
* taken through the node structure.
* 
* @param pcopy - reference to pointer which serves as the "starting
* point".
* @param pcopy_end - reference to pointer which serves as the "end point".
*/
template<typename T>
inline void linked_node<T>::copy_to(linked_node<T>*& pcopy, linked_node<T>*& pcopy_end) {
    //shit
    std::map<uptr, int> node_data;
    pcopy = new linked_node<T>[this->total_node_count()];

    int i = 0;
    auto get_node_data = [&](linked_node<T>* n1, linked_node<T>* n2) {

        uptr a1 = reinterpret_cast<uptr>(n1);
        uptr a2 = reinterpret_cast<uptr>(n2);

        if (node_data.count(a1) == 0) {
            node_data.insert(std::pair<uptr, int>(a1, i));
            pcopy[i] = linked_node<T>(n1->data);
            i++;
        }

        if (node_data.count(a2) == 0) {
            node_data.insert(std::pair<uptr, int>(a2, i));
            pcopy[i] = linked_node<T>(n2->data);
            i++;
        }

        auto& a = pcopy[node_data.at(a1)];
        auto& b = pcopy[node_data.at(a2)];
        a.add_node(b);
    };
    
    this->execute_func(get_node_data);

    pcopy_end = pcopy + i;
    return;
}


#endif // !LINKED_NODE_HPP
