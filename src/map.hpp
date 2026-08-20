/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less<Key>
> class map {
public:
    typedef pair<const Key, T> value_type;

    static const bool RED   = true;
    static const bool BLACK = false;

private:
    struct Node {
        Node *ch[2];
        Node *parent;
        bool color;
        void *data;

        // Default constructor - for nil node
        Node() : parent(this), color(BLACK), data(nullptr) {
            ch[0] = ch[1] = this;
        }

        // Constructor for data node - children set to nil later
        explicit Node(const value_type &d)
            : parent(nullptr), color(RED), data(new value_type(d)) {
            ch[0] = ch[1] = nullptr;
        }

        ~Node() {
            if (data) {
                delete static_cast<value_type *>(data);
                data = nullptr;
            }
        }

        value_type &get_value() {
            return *static_cast<value_type *>(data);
        }

        const value_type &get_value() const {
            return *static_cast<value_type *>(data);
        }
    };

    Node *nil_;   // sentinel: nil_->parent = root, nil_->ch[0] = leftmost, nil_->ch[1] = rightmost
    Node *root_;  // root of the tree (or nil_ if empty)
    size_t sz_;
    Compare cmp_;

    // Update nil_'s pointers to reflect current tree state
    void update_nil() {
        if (root_ == nil_) {
            nil_->ch[0] = nil_->ch[1] = nil_;
        } else {
            nil_->ch[0] = minimum(root_);
            nil_->ch[1] = maximum(root_);
        }
    }

    // Left rotation around x
    void rotate_left(Node *x) {
        Node *y = x->ch[1];
        x->ch[1] = y->ch[0];
        if (y->ch[0] != nil_) y->ch[0]->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->ch[0]) {
            x->parent->ch[0] = y;
        } else {
            x->parent->ch[1] = y;
        }
        y->ch[0] = x;
        x->parent = y;
    }

    // Right rotation around x
    void rotate_right(Node *x) {
        Node *y = x->ch[0];
        x->ch[0] = y->ch[1];
        if (y->ch[1] != nil_) y->ch[1]->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->ch[1]) {
            x->parent->ch[1] = y;
        } else {
            x->parent->ch[0] = y;
        }
        y->ch[1] = x;
        x->parent = y;
    }

    // Fix RB tree after insertion
    void insert_fixup(Node *z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->ch[0]) {
                Node *y = z->parent->parent->ch[1];
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->ch[1]) {
                        z = z->parent;
                        rotate_left(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_right(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->ch[0];
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->ch[0]) {
                        z = z->parent;
                        rotate_right(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_left(z->parent->parent);
                }
            }
        }
        root_->color = BLACK;
    }

    // Transplant node u with node v
    void transplant(Node *u, Node *v) {
        if (u->parent == nil_) {
            root_ = v;
        } else if (u == u->parent->ch[0]) {
            u->parent->ch[0] = v;
        } else {
            u->parent->ch[1] = v;
        }
        v->parent = u->parent;
    }

    // Find minimum node in subtree
    Node *minimum(Node *x) const {
        while (x->ch[0] != nil_) x = x->ch[0];
        return x;
    }

    // Find maximum node in subtree
    Node *maximum(Node *x) const {
        while (x->ch[1] != nil_) x = x->ch[1];
        return x;
    }

    // Fix RB tree after deletion
    void delete_fixup(Node *x) {
        while (x != root_ && x->color == BLACK) {
            if (x == x->parent->ch[0]) {
                Node *w = x->parent->ch[1];
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_left(x->parent);
                    w = x->parent->ch[1];
                }
                if (w->ch[0]->color == BLACK && w->ch[1]->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->ch[1]->color == BLACK) {
                        w->ch[0]->color = BLACK;
                        w->color = RED;
                        rotate_right(w);
                        w = x->parent->ch[1];
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->ch[1]->color = BLACK;
                    rotate_left(x->parent);
                    x = root_;
                }
            } else {
                Node *w = x->parent->ch[0];
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_right(x->parent);
                    w = x->parent->ch[0];
                }
                if (w->ch[1]->color == BLACK && w->ch[0]->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->ch[0]->color == BLACK) {
                        w->ch[1]->color = BLACK;
                        w->color = RED;
                        rotate_left(w);
                        w = x->parent->ch[0];
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->ch[0]->color = BLACK;
                    rotate_right(x->parent);
                    x = root_;
                }
            }
        }
        x->color = BLACK;
    }

    // Deep copy subtree
    Node *copy_subtree(Node *src, Node *src_nil) {
        if (src == src_nil) return nil_;
        Node *dst = new Node(src->get_value());
        dst->color = src->color;
        dst->ch[0] = copy_subtree(src->ch[0], src_nil);
        if (dst->ch[0] != nil_) dst->ch[0]->parent = dst;
        dst->ch[1] = copy_subtree(src->ch[1], src_nil);
        if (dst->ch[1] != nil_) dst->ch[1]->parent = dst;
        return dst;
    }

    // Delete subtree
    void destroy_subtree(Node *x) {
        if (x == nil_) return;
        destroy_subtree(x->ch[0]);
        destroy_subtree(x->ch[1]);
        delete x;
    }

    // Find predecessor of a node (returns nil_ if no predecessor)
    Node *predecessor(Node *x) const {
        if (x->ch[0] != nil_) return maximum(x->ch[0]);
        Node *y = x->parent;
        while (y != nil_ && x == y->ch[0]) {
            x = y;
            y = y->parent;
        }
        return y;
    }

    // Find successor of a node (returns nil_ if no successor)
    Node *successor(Node *x) const {
        if (x->ch[1] != nil_) return minimum(x->ch[1]);
        Node *y = x->parent;
        while (y != nil_ && x == y->ch[1]) {
            x = y;
            y = y->parent;
        }
        return y;
    }

public:
    class const_iterator;
    class iterator {
        private:
            Node *node_;
            const map *container_;

            friend class map;
            friend class const_iterator;

        public:
            iterator() : node_(nullptr), container_(nullptr) {}

            iterator(const iterator &other) : node_(other.node_), container_(other.container_) {}

            iterator(Node *node, const map *container)
                : node_(node), container_(container) {}

            iterator operator++(int) {
                iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            iterator &operator++() {
                if (node_ == container_->nil_ || node_ == nullptr)
                    throw invalid_iterator();
                node_ = container_->successor(node_);
                return *this;
            }

            iterator operator--(int) {
                iterator tmp(*this);
                --(*this);
                return tmp;
            }

            iterator &operator--() {
                if (container_ == nullptr)
                    throw invalid_iterator();
                if (node_ == container_->nil_) {
                    // end()-- should give last element
                    if (container_->root_ == container_->nil_)
                        throw invalid_iterator();
                    node_ = container_->maximum(container_->root_);
                    return *this;
                }
                Node *pred = container_->predecessor(node_);
                if (pred == container_->nil_)
                    throw invalid_iterator();
                node_ = pred;
                return *this;
            }

            value_type &operator*() const {
                if (node_ == container_->nil_ || node_ == nullptr)
                    throw invalid_iterator();
                return node_->get_value();
            }

            bool operator==(const iterator &rhs) const {
                return node_ == rhs.node_;
            }

            bool operator==(const const_iterator &rhs) const;

            bool operator!=(const iterator &rhs) const {
                return node_ != rhs.node_;
            }

            bool operator!=(const const_iterator &rhs) const;

            value_type *operator->() const noexcept {
                if (node_ == container_->nil_ || node_ == nullptr)
                    return nullptr;
                return &(node_->get_value());
            }
    };

    class const_iterator {
        private:
            const Node *node_;
            const map *container_;

            friend class map;
            friend class iterator;

        public:
            const_iterator() : node_(nullptr), container_(nullptr) {}

            const_iterator(const const_iterator &other)
                : node_(other.node_), container_(other.container_) {}

            const_iterator(const iterator &other)
                : node_(other.node_), container_(other.container_) {}

            const_iterator(const Node *node, const map *container)
                : node_(node), container_(container) {}

            const_iterator operator++(int) {
                const_iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            const_iterator &operator++() {
                if (node_ == container_->nil_ || node_ == nullptr)
                    throw invalid_iterator();
                node_ = const_cast<const Node *>(container_->successor(const_cast<Node *>(node_)));
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator tmp(*this);
                --(*this);
                return tmp;
            }

            const_iterator &operator--() {
                if (container_ == nullptr)
                    throw invalid_iterator();
                if (node_ == container_->nil_) {
                    if (container_->root_ == container_->nil_)
                        throw invalid_iterator();
                    node_ = const_cast<const Node *>(container_->maximum(container_->root_));
                    return *this;
                }
                Node *pred = container_->predecessor(const_cast<Node *>(node_));
                if (pred == container_->nil_)
                    throw invalid_iterator();
                node_ = const_cast<const Node *>(pred);
                return *this;
            }

            const value_type &operator*() const {
                if (node_ == container_->nil_ || node_ == nullptr)
                    throw invalid_iterator();
                return node_->get_value();
            }

            bool operator==(const const_iterator &rhs) const {
                return node_ == rhs.node_;
            }

            bool operator==(const iterator &rhs) const {
                return node_ == rhs.node_;
            }

            bool operator!=(const const_iterator &rhs) const {
                return node_ != rhs.node_;
            }

            bool operator!=(const iterator &rhs) const {
                return node_ != rhs.node_;
            }

            const value_type *operator->() const noexcept {
                if (node_ == container_->nil_ || node_ == nullptr)
                    return nullptr;
                return &(node_->get_value());
            }
    };

    map() : sz_(0) {
        nil_ = new Node();
        root_ = nil_;
    }

    map(const map &other) : sz_(other.sz_), cmp_(other.cmp_) {
        nil_ = new Node();
        if (other.root_ == other.nil_) {
            root_ = nil_;
        } else {
            root_ = copy_subtree(other.root_, other.nil_);
            root_->parent = nil_;
        }
        nil_->parent = root_;
        update_nil();
    }

    map &operator=(const map &other) {
        if (this == &other) return *this;
        clear();
        cmp_ = other.cmp_;
        sz_ = other.sz_;
        if (other.root_ != other.nil_) {
            root_ = copy_subtree(other.root_, other.nil_);
            root_->parent = nil_;
        }
        nil_->parent = root_;
        update_nil();
        return *this;
    }

    ~map() {
        destroy_subtree(root_);
        delete nil_;
    }

    T &at(const Key &key) {
        Node *node = find_node(key);
        if (node == nil_)
            throw index_out_of_bound();
        return node->get_value().second;
    }

    const T &at(const Key &key) const {
        Node *node = find_node(key);
        if (node == nil_)
            throw index_out_of_bound();
        return node->get_value().second;
    }

    T &operator[](const Key &key) {
        Node *node = find_node(key);
        if (node != nil_) return node->get_value().second;
        // Insert default value
        value_type val(key, T());
        auto result = insert(val);
        return result.first.node_->get_value().second;
    }

    const T &operator[](const Key &key) const {
        return at(key);
    }

    iterator begin() {
        return iterator(nil_->ch[0], this);
    }

    const_iterator cbegin() const {
        return const_iterator(nil_->ch[0], this);
    }

    iterator end() {
        return iterator(nil_, this);
    }

    const_iterator cend() const {
        return const_iterator(nil_, this);
    }

    bool empty() const {
        return sz_ == 0;
    }

    size_t size() const {
        return sz_;
    }

    void clear() {
        destroy_subtree(root_);
        root_ = nil_;
        sz_ = 0;
        nil_->ch[0] = nil_->ch[1] = nil_;
    }

    pair<iterator, bool> insert(const value_type &value) {
        // Search for insertion point
        Node *y = nil_;
        Node *x = root_;
        while (x != nil_) {
            y = x;
            if (cmp_(value.first, x->get_value().first)) {
                x = x->ch[0];
            } else if (cmp_(x->get_value().first, value.first)) {
                x = x->ch[1];
            } else {
                // Key already exists
                return pair<iterator, bool>(iterator(x, this), false);
            }
        }

        Node *z = new Node(value);
        z->ch[0] = z->ch[1] = nil_;
        z->parent = y;
        if (y == nil_) {
            root_ = z;
            nil_->parent = root_;
        } else if (cmp_(value.first, y->get_value().first)) {
            y->ch[0] = z;
        } else {
            y->ch[1] = z;
        }

        insert_fixup(z);
        sz_++;
        update_nil();
        return pair<iterator, bool>(iterator(z, this), true);
    }

    void erase(iterator pos) {
        if (pos.node_ == nil_ || pos.node_ == nullptr || pos.container_ != this)
            throw invalid_iterator();
        Node *z = pos.node_;
        erase_node(z);
    }

    size_t count(const Key &key) const {
        return find_node(key) != nil_ ? 1 : 0;
    }

    iterator find(const Key &key) {
        Node *node = find_node(key);
        return iterator(node, this);
    }

    const_iterator find(const Key &key) const {
        Node *node = find_node(key);
        return const_iterator(node, this);
    }

private:
    // Find node with key
    Node *find_node(const Key &key) const {
        Node *x = root_;
        while (x != nil_) {
            if (cmp_(key, x->get_value().first)) {
                x = x->ch[0];
            } else if (cmp_(x->get_value().first, key)) {
                x = x->ch[1];
            } else {
                return x;
            }
        }
        return nil_;
    }

    // Erase a node from the tree
    void erase_node(Node *z) {
        Node *y = z;
        Node *x;
        bool y_original_color = y->color;

        if (z->ch[0] == nil_) {
            x = z->ch[1];
            transplant(z, z->ch[1]);
        } else if (z->ch[1] == nil_) {
            x = z->ch[0];
            transplant(z, z->ch[0]);
        } else {
            y = minimum(z->ch[1]);
            y_original_color = y->color;
            x = y->ch[1];
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->ch[1]);
                y->ch[1] = z->ch[1];
                y->ch[1]->parent = y;
            }
            transplant(z, y);
            y->ch[0] = z->ch[0];
            y->ch[0]->parent = y;
            y->color = z->color;
        }
        delete z;
        sz_--;
        if (y_original_color == BLACK) {
            delete_fixup(x);
        }
        nil_->parent = root_;
        update_nil();
    }
};

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator==(const typename map<Key, T, Compare>::const_iterator &rhs) const {
    return node_ == rhs.node_;
}

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator!=(const typename map<Key, T, Compare>::const_iterator &rhs) const {
    return node_ != rhs.node_;
}

}

#endif
