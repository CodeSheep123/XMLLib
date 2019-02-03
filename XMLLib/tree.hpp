#ifndef MVG_XML_TREE_HPP_
#define MVG_XML_TREE_HPP_

#include "exception.hpp"
#include "index.hpp"

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#ifdef _DEBUG
#    include <iostream>
#endif

namespace mvg {

namespace xml {

template<typename T>
class tree {
public:
    struct node {
        node() : data{}, children{}, parent(nullptr) {}
        node(T const& d, node* p) {
            data = d;
            parent = p;
        }

        node(node const&) = default;
        node(node&&) = default;
        node& operator=(node const&) = default;
        node& operator=(node&&) = default;

        T data;

        std::vector<node> children;
        // The parent of the node. nullptr if this node is the root node
        node* parent;

        // Return this node's siblings, including this node
        std::vector<node>& siblings() {
            if (parent == nullptr) {
                throw nullptr_exception(
                    "Cannot retrieve siblings of the root node");
            }
            return parent->children;
        }

        // Return this node's siblings, including this node
        std::vector<node> const& siblings() const {
            if (parent == nullptr) {
                throw nullptr_exception(
                    "Cannot retrieve siblings of the root node");
            }
            return parent->children;
        }
    };

    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;

    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    tree() : root_{T{}, nullptr} {
        static_assert(std::is_default_constructible<T>::value,
                      "Type T must be default constructible to use default "
                      "constructor for class tree");
    }

    tree(T root_) : root_{root_, nullptr} {}
    tree(tree const& rhs) {
        root_ = rhs.root_;
        set_parents(&root_);
    }

    tree(tree&& rhs) { root_ = std::move(rhs.root_); }

    tree& operator=(tree const& rhs) {
        root_ = rhs.root_;
        set_parents(&root_);
        return *this;
    }

    tree& operator=(tree&& rhs) { root_ = std::move(rhs.root_); }

    ~tree() = default;

    node* root() { return &root_; }

    node const* root() const { return &root_; }

    template<typename U>
    node* insert(node* parent, U&& data) {
        static_assert(std::is_convertible<U, T>::value,
                      "U must be convertible to T");
        return &parent->children.emplace_back(std::forward<U>(data), parent);
    }

    template<typename U>
    node* find(U&& data) {
        node* n = nullptr;
        for_each_node([&n, &data](node* cur) {
            if (cur->data == data) { n = cur; }
        });
        return n;
    }

    template<typename U>
    node const* find(U&& data) const {
        node const* n = nullptr;
        for_each_node([&n, &data](node const* cur) {
            if (cur->data == data) { n = cur; }
        });
        return n;
    }

    // Deletes the node and all its children
    void erase(node* node) {
        node->children.clear();
        node->parent->children.erase(std::remove(node->parent->children.begin(),
                                                 node->parent->children.end(),
                                                 *node));
    }

    // Takes a predicate, signature must be bool(node*)
    template<typename F>
    void erase_if(F&& f) {
        for_each_node([&f](node* node) { if (f(node) { erase(node); }) });
    }

#ifdef _DEBUG
    void print_as_tree_impl(std::ostream& out, node* cur, std::size_t level) {
        // Indent the element properly
        for (std::size_t i = 1; i < level; ++i) out << "         ";
        if (level != 0) {
            out << "|";
            out << "------- ";
        }
        // Print the node
        out << cur->data << "\n";

        for (auto& child : cur->children) {
            print_as_tree_impl(out, &child, level + 1);
        }

        out.flush();
    }

    // Prints the tree in a treelike structure, and flushes the output stream
    void print_as_tree(std::ostream& out) {
        print_as_tree_impl(out, &root_, 0);
    }
#endif

private:
    node root_;

    void set_parents(node* cur) {
        for (auto& child : children) {
            if (cur != &root_) { child.parent = cur; }
            set_parents(&child);
        }
    }

    template<typename F>
    void for_each_node_impl(node* cur, F&& func) {
        func(cur);
        for (auto& child : cur->children) { for_each_node_impl(&child, func); }
    }

    template<typename F>
    void for_each_node(F&& func) {
        for_each_node_impl(&root_, std::forward<F>(func));
    }

    // const version

    template<typename F>
    void for_each_node_impl(node const* cur, F&& func) const {
        func(cur);
        for (auto& child : cur->children) { for_each_node_impl(&child, func); }
    }

    template<typename F>
    void for_each_node(F&& func) const {
        for_each_node_impl(&root_, std::forward<F>(func));
    }
};

namespace detail {

template<typename T, typename F>
void for_each_impl(typename tree<T>::node* cur, F&& func) {
    func(cur->data);
    for (auto& child : cur->children) {
        for_each_impl<T, F>(&child, std::forward<F>(func));
    }
}

// const version
template<typename T, typename F>
void for_each_impl(typename tree<T>::node const* cur, F&& func) {
    func(cur->data);
    for (auto& child : cur->children) {
        for_each_impl<T, F>(&child, std::forward<F>(func));
    }
}

} // namespace detail

template<typename T, typename F>
void for_each(tree<T>& t, F&& func) {
    detail::for_each_impl<T, F>(t.root(), std::forward<F>(func));
}

// const version
template<typename T, typename F>
void for_each(tree<T> const& t, F&& func) {
    detail::for_each_impl<T, F>(t.root(), std::forward<F>(func));
}

} // namespace xml

} // namespace mvg

#endif
