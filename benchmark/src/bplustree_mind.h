#ifndef PC_API_BPLUSTREE_H
#define PC_API_BPLUSTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <iostream>
#include <vector>
#include <queue> 
#include <fstream>
#include "mind_malloc.h"

long char_to_int64(char* a) {
    long num = 0;
    for ( int i = 0 ; i < sizeof(a) ; i++ )
        num = (num << 8) | a[i];
    return num;
}

/* minimum degree - every node (except root) must contain (BPLUSTREE_MIN_DEGREE - 1) keys */
/* all nodes (including root) may contain at most (2*BPLUSTREE_MIN_DEGREE - 1) keys */
#define BPLUSTREE_MIN_DEGREE 5

/* maximum keys a node can hold */
#define BPLUSTREE_MAX_KEYS (2 * BPLUSTREE_MIN_DEGREE - 1)

/* maximum children a node can have */
#define BPLUSTREE_MAX_CHILDREN (2 * BPLUSTREE_MIN_DEGREE)

/* default length for value */
#define DEFAULT_VALUE_LEN 8

#define LEAF_NODE_TRUE_FLAG 1
#define LEAF_NODE_FALSE_FLAG 0

// verbose
//#define VERBOSE_FLAG

/* types of allocations */
enum bplustree_alloc_types {
    BPLUSTREE_NODE_TYPE,
    BPLUSTREE_ENTRY_TYPE,
    BPLUSTREE_MAX_TYPES
};

/* declaration of data-structures */

/*structure size: 16 Bytes*/
struct bplustree_entry {
    uint64_t key;   //8 Bytes
    char value[DEFAULT_VALUE_LEN];  //8 Bytes
    bplustree_entry(uint64_t _key, char* _value) {
        key = _key;
        for(int i = 0; i < DEFAULT_VALUE_LEN; i++) {
            value[i] = _value[i];
        }
    }
    bplustree_entry(const bplustree_entry & entry) {
// #ifdef VERBOSE_FLAG
//         std::cout << "calling copy constructor " << std::endl;
// #endif
        key = entry.key;
        for(int i = 0; i < DEFAULT_VALUE_LEN; i++) {
            value[i] = entry.value[i];
        }
    }
    bplustree_entry & operator=(const bplustree_entry & entry) {
// #ifdef VERBOSE_FLAG
//         std::cout << "calling operator overwrite function " << std::endl;
//         std::cout << "new key: " << entry.key << std::endl;
// #endif
        key = entry.key;
        for(int i = 0; i < DEFAULT_VALUE_LEN; i++) {
            value[i] = entry.value[i];
        }
        return *this;
    }
} __attribute__ ((aligned (8)));

/*structure size: {24 + (144 * BPLUSTREE_MAX_KEYS) + 8} Bytes*/
struct bplustree_node {
    //flag to check if the node is leaf or not
    long is_leaf;    //4 Bytes

    //current number of keys
    long nk; //4 Bytes

    //array of <key-value> entries
    struct bplustree_entry entries[BPLUSTREE_MAX_KEYS];     //(16 * BPLUSTREE_MAX_KEYS) Bytes

    //array of child pointer
    struct bplustree_node *children[BPLUSTREE_MAX_CHILDREN];   //(8 * BPLUSTREE_MAX_CHILDREN) Bytes

    //used for leaf type nodes
    struct bplustree_node *next;   //8 Bytes
    // struct bplustree_node *previous;   //8 Bytes
};

class BPlusTree {
public:
    BPlusTree(const char *path) {
        BPlusTree::init(path);
    }

    int init(const char *path);

    void *get_root(void);

    int set_root(void *root_ptr);

    int read(const uint64_t key, void *&result);

    int scan(const uint64_t key, int len, std::vector<struct bplustree_entry> &result);

    int aggregate(const uint64_t key, unsigned long long len, long &result, int agg_type, int verbose);

    int update(const uint64_t key, void *value, bool is_char);

    int insert(const uint64_t key, void *value, bool is_char);

    int debug_traverse_all(void);

    void destroy();

    ~BPlusTree() {
        destroy();
    }

    void print_layout(std::string filepath, bool size_only); 

private:
    struct bplustree_node *root;
    std::vector<bplustree_node*> nodes; 

    int check();

    int is_node_full(int nk);

    struct bplustree_node *create_node(int _is_leaf);

    char *search(struct bplustree_node *current_node, uint64_t key);

    void split_node(int idx, struct bplustree_node *parent, struct bplustree_node *child);

    void insert_not_full(struct bplustree_node *node, uint64_t key, void *value, bool is_char);

    bool update_if_found(struct bplustree_node *current_node, uint64_t key, void *value, bool is_char);

    void recursive_free(struct bplustree_node *current_node);

    void print_leaf_chain();
};

/**
 * BPlusTree::check -- (internal) checks if bplus tree has been initialized
 */
inline int BPlusTree::check() {
    if (root == NULL) {
        fprintf(stderr, "[%s]: FATAL: bplustree not initialized yet\n", __FUNCTION__);
        assert(0);
    }
    return 1;
}

/**
 * BPlusTree::print_leaf_chain -- (internal) print the leaf chain to check the integrity of bplus-tree
 */
void BPlusTree::print_leaf_chain() {
    struct bplustree_node *current_node = root;
    while (current_node != NULL && !current_node->is_leaf) {
        current_node = current_node->children[0];
    }

    while (current_node != NULL) {
        printf("[");
        for (int i = 0; i < current_node->nk; i += 1) {
            printf(" <%ld, %s>", current_node->entries[i].key, current_node->entries[i].value);
        }
        printf("] -> ");
        current_node = current_node->next;
    }
    printf("\n");
}

/**
 * BPlusTree::is_node_full -- (internal) checks if bplus tree node contains max possible <key-value> pairs
 */
inline int BPlusTree::is_node_full(int nk) {
    return nk == BPLUSTREE_MAX_KEYS ? 1 : 0;
}

/**
 * BPlusTree::init -- initialize bplus tree
 */
int BPlusTree::init(const char *path) {
    root = NULL;
    return 1;
}

void *BPlusTree::get_root(void)
{
    return (void *)root;
}

int BPlusTree::set_root(void *root_ptr) {
    root = (bplustree_node *)root_ptr;
    return 1;
}

/**
 * BPlusTree::scan -- perform range query
 */
int BPlusTree::scan(const uint64_t key, int len, std::vector <struct bplustree_entry> &result) {
    check();

    struct bplustree_node *current_node = root;
    // struct bplustree_node *tmp = (struct bplustree_node *)malloc(sizeof(*tmp));
    int total_chase = 0;

    //going down upto the leaf
    while(current_node != NULL && !current_node->is_leaf) {
        int i = 0;
        while (i < current_node->nk && key > current_node->entries[i].key) {
            i += 1;
        }
        current_node = current_node->children[i];
        // total_chase ++;
    }

    //reached the leaf
    if (current_node) {
        int i = 0;
        //find the starting point to start the scan
        while (i < current_node->nk && key > current_node->entries[i].key) {
            i += 1;
        }

        int len_count = 0;
        //forward scan through the leaf chain
#ifdef VERBOSE_FLAG
        std::cout << "new scan ::";
#endif
        while(current_node != NULL) {
#ifdef VERBOSE_FLAG
            std::cout << (unsigned long)current_node << " -> ";
#endif
            while (i < current_node->nk) {
                result.push_back(bplustree_entry(current_node->entries[i].key, current_node->entries[i].value));

                len_count += 1;
                if(len_count == len) break;
                i += 1;
            }
            if (len_count == len) break;
            // memcpy(tmp, current_node, sizeof(*tmp));
            current_node = current_node->next;
            total_chase ++;
            i = 0;
        }
#ifdef VERBOSE_FLAG
        std::cout << std::endl;
#endif
    }
    // free(tmp);
    return total_chase;
}

int BPlusTree::debug_traverse_all(void) {
    check();

    struct bplustree_node *current_node = root;
    struct bplustree_node *tmp = (struct bplustree_node *)malloc(sizeof(*tmp));
    unsigned long long total_leaves = 0;

    //going down upto the left most leaf
    while(current_node != NULL && !current_node->is_leaf) {
        current_node = current_node->children[0];
    }

    //reached the leaf
    while(current_node != NULL) {
        memcpy(tmp, current_node, sizeof(*tmp));
        current_node = current_node->next;
        total_leaves ++;
    }

    std::cout << "Total leaves: " << total_leaves << std::endl;
    free(tmp);
    return 0;
}

/**
 * BPlusTree::scan -- perform range query
 * agg_type
 * 0 sum
 * 1 count
 * 2 max
 * 3 min
 */
int BPlusTree::aggregate(const uint64_t key, unsigned long long len, long &result, int agg_type, int verbose = false) {
    check();

    struct bplustree_node *current_node = root;
    int total_chase = 0;

    //going down upto the leaf
    while(current_node != NULL && !current_node->is_leaf) {
        int i = 0;

        while (i < current_node->nk && key > current_node->entries[i].key) {
            i += 1;
        }
        current_node = current_node->children[i];
    }

    //reached the leaf
    if(current_node != NULL) {
        int i = 0;
        //find the starting point to start the scan
        while (i < current_node->nk && key > current_node->entries[i].key) {
            i += 1;
        }

        //forward scan through the leaf chain
        while(current_node != NULL) {
            while (i < current_node->nk) {
                long node_value = char_to_int64(current_node->entries[i].value);
                switch (agg_type) {
                    case 0:
                        result += node_value; 
                    break;  
                    case 1:
                        result += 1;
                    break;
                    case 2:
                        if(node_value > result) {
                            result = node_value;                        
                        }
                    break;  
                    case 3:
                        if(node_value < result) {
                            result = node_value;
                        }
                    break;
                }
                if (current_node->entries[i].key > key + len) break;
                i++;
            }
            if (verbose)
                std::cout << key << "->";
            if (current_node->entries[i].key > key + len) break;
            current_node = current_node->next;
            total_chase ++;
            i = 0;
        }
        if (verbose)
            std::cout << std::endl;
    }

    return total_chase;
}

/**
 * BPlusTree::search -- (internal) search the node contains the key and return the value
 */
//char *BPlusTree::search(struct bplustree_node *current_node, uint64_t key) {
//#ifdef VERBOSE_FLAG
//    std::cout << "Printing the Node info" << std::endl;
//    std::cout << "Is it a leaf? " << current_node->is_leaf << std::endl;
//    std::cout << "number of keys: " << current_node->nk << std::endl;
//    for (int j = 0; j < current_node->nk; j++) {
//        std::cout << current_node->entries[j].key << " " << current_node->entries[j].value << std::endl;
//    }
//#endif
//
//    while(current_node != NULL && !current_node->is_leaf) {
//        int i = 0;
//
//        while (i < current_node->nk && key > current_node->entries[i].key) {
//            i += 1;
//        }
//        current_node = current_node->children[i];
//#ifdef VERBOSE_FLAG
//        std::cout << "Going to the next node!" << std::endl;
//#endif
//    }
//
//
//    ////todo: it is possible to apply binary search here to make the search faster
//    //while (i < current_node->nk && key > current_node->entries[i].key) {
//    //    i += 1;
//    //}
//
//    //we reached to leaf
//#ifdef VERBOSE_FLAG
//    std::cout << "Reaching the leaf!" << std::endl;
//    std::cout << "Printing the leaf info" << std::endl;
//    std::cout << "Is it a leaf? " << current_node->is_leaf << std::endl;
//    std::cout << "number of keys: " << current_node->nk << std::endl;
//    for (int j = 0; j < current_node->nk; j++) {
//        std::cout << current_node->entries[j].key << " " << current_node->entries[j].value << std::endl;
//    }
//#endif
//
//    int i = 0;
//    while (i < current_node->nk && key > current_node->entries[i].key) {
//        i += 1;
//    }
//
//    if (current_node->is_leaf) {
//        //check if we found the key
//#ifdef VERBOSE_FLAG
//        std::cout << "Checking entry: " << i << " and key is: " << current_node->entries[i].key << std::endl;
//#endif
//        if (i < current_node->nk && key == current_node->entries[i].key) {
//            //key found, return the value
//            return current_node->entries[i].value;
//        }
//        //key not found
//        return NULL;
//    }
//
//    //the node is not leaf, move to the proper child node
//    if (i < current_node->nk && key == current_node->entries[i].key) i += 1;
//    return search(current_node->children[i], key);
//}

/**
 * BPlusTree::search -- (internal) search the node contains the key and return the value
 */
char *BPlusTree::search(struct bplustree_node *current_node, uint64_t key) {
    int i = 0;

    //todo: it is possible to apply binary search here to make the search faster
    while (i < current_node->nk && key > current_node->entries[i].key) {
        i += 1;
    }

    //we reached to leaf
    if (current_node->is_leaf) {
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << "Printing the Node info" << std::endl;
        std::cout << "Is it a leaf? " << current_node->is_leaf << std::endl;
        std::cout << "number of keys: " << current_node->nk << std::endl;
        for (int j = 0; j < current_node->nk; j++) {
            std::cout << current_node->entries[j].key << " " << current_node->entries[j].value << " " << &(current_node->entries[j].value) << std::endl;
        }
#endif
        //check if we found the key
        if (i < current_node->nk && key == current_node->entries[i].key) {
            //key found, return the value
            return current_node->entries[i].value;
        }
        //key not found
        return NULL;
    }

    //the node is not leaf, move to the proper child node
    if (i < current_node->nk && key == current_node->entries[i].key) i += 1;
    return search(current_node->children[i], key);
}

/**
 * BPlusTree::read -- read 'value' of 'key' from btree and place it into '&result'
 */
int BPlusTree::read(const uint64_t key, void *&result) {
    check();
    result = search(root, key);
    return 1;
}

/**
 * BPlusTree::update -- update 'value' of 'key' into btree, will insert the 'value' if 'key' not exists
 */
int BPlusTree::update(const uint64_t key, void *value, bool is_char = true) {
    //printf("[%s]: PARAM: key: %s, value: %s\n", __func__, key, (char *) value);
    check();
    return insert(key, value, is_char);
}

/**
 * BPlusTree::create_node -- (internal) create new bplus tree node
 */
inline struct bplustree_node *BPlusTree::create_node(int _is_leaf) {
    // struct bplustree_node *new_node_p = (struct bplustree_node *) malloc(sizeof(struct bplustree_node));
    struct bplustree_node *new_node_p = (struct bplustree_node *)mind_malloc(sizeof(struct bplustree_node));
    new_node_p->is_leaf = _is_leaf;
    new_node_p->nk = 0;
    new_node_p->next = NULL;    // new_node_p->previous = NULL;

    nodes.push_back(new_node_p);    // DEBUG/FINAL PRINT: we can keep it local
    return new_node_p;
}

/**
 * BPlusTree::update_if_found -- (internal) search the key and if exist, update the value
 */
bool BPlusTree::update_if_found(struct bplustree_node *current_node, uint64_t key, void *value, bool is_char = true) {
    int i = 0;

    //todo: it is possible to apply binary search here to make the search faster
    while (i < current_node->nk && key > current_node->entries[i].key) {
        i += 1;
    }

    //we reached to leaf
    if (current_node->is_leaf) {
        //check if we found the key
        if (i < current_node->nk && key == current_node->entries[i].key) {
            //key found, update value and return
            if (is_char)
                memcpy(current_node->entries[i].value, (char *)value,
                    std::min((size_t)DEFAULT_VALUE_LEN, strlen((char *)value) + 1));
            else
                memcpy(current_node->entries[i].value, (char *)value, DEFAULT_VALUE_LEN);
            return true;
        }
        //key not found
        return false;
    }

    //the node is not leaf, move to the proper child node
    if (i < current_node->nk && key == current_node->entries[i].key) i += 1;
    return update_if_found(current_node->children[i], key, value, is_char);
}

/**
 * BPlusTree::split_node -- (internal) split the children of the child node equally with the new sibling node
 *
 * so, after this split, both the child and sibling node will hold BPLUSTREE_MIN_DEGREE children,
 * one children will be pushed to the parent node.
 *
 * this function will be called when the child node is full and become idx'th child of the parent,
 * the new sibling node will be the (idx+1)'th child of the parent.
 */
void BPlusTree::split_node(int idx, struct bplustree_node *parent, struct bplustree_node *child) {
    if (child->is_leaf) {
        //new right-sibling node will get the same status as child
        struct bplustree_node *sibling = create_node(child->is_leaf);
        //new right-sibling node will hold the BPLUSTREE_MIN_DEGREE entries of child node
        sibling->nk = BPLUSTREE_MIN_DEGREE;

        //transfer the last BPLUSTREE_MIN_DEGREE entries of child node to it's sibling node
        for (int i = 0; i < BPLUSTREE_MIN_DEGREE; i += 1) {
            sibling->entries[i] = child->entries[i + BPLUSTREE_MIN_DEGREE - 1];
        }

        //reduce the number of entries of child node
        child->nk = BPLUSTREE_MIN_DEGREE - 1;

        //as parent node is going to get a new child at (idx+1)-th place, make a room for it
        for (int i = parent->nk; i >= idx + 1; i -= 1) {
            parent->children[i + 1] = parent->children[i];
        }

        //place sibling node as parent's children
        parent->children[idx + 1] = sibling;

        //a entry of child node will move to the parent node, make a room for it
        for (int i = parent->nk - 1; i >= idx; i -= 1) {
            parent->entries[i + 1] = parent->entries[i];
        }

        //place the 0'th key of sibling node to parent node
        parent->entries[idx].key = sibling->entries[0].key;

        //parent now hold a new key, so increasing the number of keys
        parent->nk += 1;

        //adjust leaf chain pointers
        sibling->next = child->next;
        // if (sibling->next != NULL) {
        //     sibling->next->previous = sibling;
        // }
        child->next = sibling;
        // sibling->previous = child;
    } else {
        //new right-sibling node will get the same status as child
        struct bplustree_node *sibling = create_node(child->is_leaf);
        //new sibling child will hold the (BPLUSTREE_MIN_DEGREE - 1) entries of child node
        sibling->nk = BPLUSTREE_MIN_DEGREE - 1;

        //transfer the last (BPLUSTREE_MIN_DEGREE - 1) entries of child node to it's sibling node
        for (int i = 0; i < BPLUSTREE_MIN_DEGREE - 1; i += 1) {
            sibling->entries[i] = child->entries[i + BPLUSTREE_MIN_DEGREE];
        }

        //as child is an internal node, transfer the last (BPLUSTREE_MIN_DEGREE) children pointers of child node to it's sibling node
        for (int i = 0; i < BPLUSTREE_MIN_DEGREE; i += 1) {
            sibling->children[i] = child->children[i + BPLUSTREE_MIN_DEGREE];
        }

        child->nk = BPLUSTREE_MIN_DEGREE - 1;

        //as parent node is going to get a new child at (idx+1)-th place, make a room for it
        for (int i = parent->nk; i >= idx + 1; i -= 1) {
            parent->children[i + 1] = parent->children[i];
        }

        //place sibling node as parent's children
        parent->children[idx + 1] = sibling;

        //a entry of child node will move to the parent node, make a room for it
        for (int i = parent->nk - 1; i >= idx; i -= 1) {
            parent->entries[i + 1] = parent->entries[i];
        }

        //place the middle entry of child node to parent node
        parent->entries[idx] = child->entries[BPLUSTREE_MIN_DEGREE - 1];

        //parent now hold a new entry, so increasing the number of keys
        parent->nk += 1;
    }
}

/**
 * BPlusTree::insert_not_full -- (internal) inserts <key, value> pair into node
 * when this function called, we can assume that the node has space to hold new data
 */
void BPlusTree::insert_not_full(struct bplustree_node *node, uint64_t key, void *value, bool is_char = true) {
    int i = node->nk - 1;

    // if node is a leaf, insert the data to actual position and return
    if (node->is_leaf) {
        while (i >= 0 && node->entries[i].key > key) {
            node->entries[i + 1] = node->entries[i];
            i -= 1;
        }
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << "Printing the Node info before insert" << std::endl;
        std::cout << "Is it a leaf? " << node->is_leaf << std::endl;
        std::cout << "number of keys: " << node->nk << std::endl;
        for (int j = 0; j < node->nk + 1; j++) {
            std::cout << node->entries[j].key << " " << node->entries[j].value << std::endl;
        }
#endif

        node->entries[i + 1].key = key;
        if (is_char)
            memcpy(node->entries[i + 1].value, (char *)value,
                std::min((size_t)DEFAULT_VALUE_LEN, strlen((char *)value) + 1));
        else
            memcpy(node->entries[i + 1].value, (char *)value, DEFAULT_VALUE_LEN);
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << "Memcpy size: " << strlen((char *) value) + 1 << std::endl;
#endif
        node->nk += 1;
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << "Printing the Node info after insert" << std::endl;
        std::cout << "Is it a leaf? " << node->is_leaf << std::endl;
        std::cout << "number of keys: " << node->nk << std::endl;
        for (int j = 0; j < node->nk; j++) {
            std::cout << node->entries[j].key << " " << node->entries[j].value << std::endl;
        }
#endif
        return;
    }

    // the node is not a leaf, find the child which is going to store the new data
    while (i >= 0 && node->entries[i].key > key) {
        i -= 1;
    }

    //check if the child is full
    if (is_node_full(node->children[i + 1]->nk)) {
        //child is full, need to split
        split_node(i + 1, node, node->children[i + 1]);

        //after the split, child's middle entry is pushed to parent
        //decide which children will hold the new <key,value> pair
        if (node->entries[i + 1].key < key) {
            i += 1;
        }
    }
    insert_not_full(node->children[i + 1], key, value ,is_char);
}

/**
 * BPlusTree::insert -- inserts <key, value> pair into bplus tree, will update the 'value' if 'key' already exists
 */
int BPlusTree::insert(const uint64_t key, void *value, bool is_char = true) {
    //printf("[%s]: PARAM: key: %ld, value: %s\n", __func__, key, (char *) value);

    // if bplus tree is empty, create root
    if (root == NULL) {
        root = create_node(LEAF_NODE_TRUE_FLAG);    //root is also a leaf
        root->entries[0].key = key;
        if (is_char)
            memcpy(root->entries[0].value, (char *)value,
                std::min((size_t)DEFAULT_VALUE_LEN, strlen((char *)value) + 1));
        else
            memcpy(root->entries[0].value, (char *)value, DEFAULT_VALUE_LEN);
        root->nk = 1;
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << "Printing the Node info after insert" << std::endl;
        std::cout << "Is it a leaf? " << root->is_leaf << std::endl;
        std::cout << "number of keys: " << root->nk << std::endl;
        for (int j = 0; j < root->nk; j++) {
            std::cout << root->entries[j].key << " " << root->entries[j].value << std::endl;
        }
#endif
        return 1;
    }

    // if the key already exist in bplus tree, update the value and return
    bool is_updated = update_if_found(root, key, value, is_char);
    if (is_updated) return 1;

    // if root is full
    if (is_node_full(root->nk)) {
        int idx = 0;
        struct bplustree_node *new_root = create_node(LEAF_NODE_FALSE_FLAG);    //root is not a leaf anymore
        new_root->children[idx] = root;
        split_node(idx, new_root, root);

        //new_root is holding two children now, decide which children will hold the new <key,value> pair
        if (new_root->entries[idx].key < key) {
            idx += 1;
        }

        //new_root->children[idx] will definitely have space now, go ahead and insert the data to proper place
        insert_not_full(new_root->children[idx], key, value, is_char);

        //update the root
        root = new_root;
    } else {
        //root is not full, go ahead and insert the data to proper place
        insert_not_full(root, key, value, is_char);
    }

    return 1;
}

/**
 * btree_recursive_free -- recursively visit all the btree nodes and de-allocate memory
 */
// void BPlusTree::recursive_free(struct bplustree_node *current_node) {
//     //base case
//     if (current_node->is_leaf) {
//         if(current_node->next != NULL) current_node->next->previous = current_node->previous;
//         if(current_node->previous != NULL) current_node->previous->next = current_node->next;
//         free(current_node);
//         return;
//     }

//     //recursively call all the child nodes
//     for (int i = 0; i < current_node->nk + 1; i += 1) {
//         recursive_free(current_node->children[i]);
//     }

//     //free the memory
//     free(current_node);
// }

/**
 * BPlusTree::destroy -- destructor
 */
void BPlusTree::destroy() {
    check();
    // recursive_free(root);
    root = NULL;
}

void BPlusTree::print_layout(std::string filepath, bool size_only = false) {
    // std::ofstream fout(filepath); 
    // int node_size = sizeof(bplustree_node);
    // long long tree_size = node_size * (long long)nodes.size(); 
    // fout << "Tree size = " << (double)tree_size/1e9 << " GB" << std::endl; 
    // if(size_only) {
    //     fout.close(); 
    //     return; 
    // }

    // unsigned long long min_addr = (unsigned long long)root; 

    // for(auto curr_node: nodes) {
    //     if(min_addr > (unsigned long long)curr_node) {
    //         min_addr = (unsigned long long)curr_node; 
    //     } 
    // }

    // for(auto curr_node: nodes) {
    //     //decrease children, next and previous pointers 
    //     for(int i = 0; i <= curr_node->nk; ++i) {
    //         curr_node->children[i] = (bplustree_node *)((unsigned long long)curr_node->children[i] - min_addr); 
    //     }
    //     if(curr_node->next != NULL) curr_node->next = (bplustree_node *)((unsigned long long)curr_node->next - min_addr); 
    //     if(curr_node->previous != NULL) curr_node->previous = (bplustree_node *)((unsigned long long)curr_node->previous - min_addr); 

    //     // print node 
    //     unsigned char* addr = (unsigned char *)curr_node; 
    //     fout << "New node: address = " << (void *)(addr - (unsigned char *)min_addr) << std::endl; 
    //     for(int i = 0; i < sizeof(bplustree_node); ++i, ++addr) {
    //         fout << "address = " << (void *)(addr - (unsigned char *)min_addr) << " value = " << std::hex << (unsigned int)*addr << std::endl; 
    //     }
    //     fout << std::endl; 

    //     //increase children next and previous pointers 
    //     for(int i = 0; i <= curr_node->nk; ++i) {
    //         curr_node->children[i] = (bplustree_node *)((unsigned long long)curr_node->children[i] + min_addr); 
    //     }
    //     if(curr_node->next != NULL) curr_node->next = (bplustree_node *)((unsigned long long)curr_node->next + min_addr); 
    //     if(curr_node->previous != NULL) curr_node->previous = (bplustree_node *)((unsigned long long)curr_node->previous + min_addr); 
    // }

    // fout.close();   
}


#endif //PC_API_BPLUSTREE_H