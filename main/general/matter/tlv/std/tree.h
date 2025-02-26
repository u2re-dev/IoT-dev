#pragma once

//
#ifndef D5260195_61AB_4BF6_AF66_AFA17C759B97
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <queue>
#include <utility>
#include <vector>
#include <cstring>

//
#include <std/types.hpp>
//#include "../tlv.h"

//
namespace tlvcpp {
    template<typename T> class tree_node {
    public:
        tree_node() {}
        template<typename... Args> tree_node(T const &value): m_parent(nullptr), m_data(value) {}
        //template<typename... Args> explicit tree_node(Args&&... args): m_parent(nullptr), m_data(std::forward<Args>(args)...) {}
        //template<typename... Args> explicit tree_node(tree_node *parent, Args&&... args): m_parent(parent), m_data(std::forward<Args>(args)...) {}
        tree_node(tree_node *parent, T const &value): m_parent(parent), m_data(value) {}
        tree_node(tree_node &other): tree_node(const_cast<const tree_node &>(other)) {}
        tree_node(tree_node &&other) noexcept: m_parent(nullptr), m_data(std::move(other.m_data)), m_children(std::move(other.m_children)) { if (other.m_parent) other.parent()->prune(other); for (auto &child : m_children) child.m_parent = this; }
        tree_node(const tree_node &other): m_parent(nullptr), m_data(other.m_data) { for (auto const &child : other.m_children) graft(child); }

        //
        tree_node &operator=(const tree_node &other) { if (this == &other) return *this; assert(!other.is_parent_of(*this)); m_data = other.m_data; for (auto const &child : other.m_children) graft(child); return *this; }
        tree_node &operator=(tree_node &&other) noexcept { if (this == &other) return *this; assert(!other.is_parent_of(*this)); m_data = std::move(other.m_data); if (is_parent_of(other)) m_children = std::move(std::list<tree_node>(std::move(other.m_children))); else { m_children = std::move(other.m_children); if (other.m_parent) other.parent()->prune(other); } for (auto &child : m_children) child.m_parent = this; return *this; }

        //
        const tree_node &root() const { const auto *node = this; while (node->parent()) node = node->parent(); return *node; }
        const tree_node *parent() const { return m_parent; }
        tree_node &root() { auto *node = this; while (node->parent()) node = node->parent(); return *node; }
        tree_node *parent() { return m_parent; }
        size_t depth() const { size_t d = 0; for (auto *node = this; node->parent(); node = node->parent()) d++; return d; }
        const T &data() const { return m_data; }
        T &data() { return m_data; }
        const std::list<tree_node> &children() const { return m_children; }
        std::list<tree_node> &children() { return m_children; }

        //
        template<typename... Args> tree_node<T> &add_child(Args&&... args) { return m_children.emplace_back(this, tlv(std::forward<Args>(args)...)); }
        tree_node<T> &add_child(tree_node<T>& node) { m_children.push_back(node); return node; }
        tree_node<T> &add_child(T const& node) { return m_children.emplace_back(this, node); }

        //
        void graft(const tree_node &node) { assert(this != &node && !node.is_parent_of(*this)); auto &grafted = add_child(node.m_data); for (auto const &child : node.m_children) grafted.graft(child); }
        void prune(tree_node &node) { auto it = std::find_if(m_children.begin(), m_children.end(), [&node](const tree_node &_node){ return &_node == &node; }); if (it != m_children.end()) m_children.erase(it); }

        //
        template<typename U> const tree_node *find(U value, size_t index = 0) const { return find_impl(value, index); }
        template<typename U> const tree_node *find_immediate(U value, size_t index = 0) const { return find_immediate_impl(value, index); }
        template<typename U> tree_node *find(U value, size_t index = 0) { return find_impl(value, index); }
        template<typename U> tree_node *find_immediate(U value, size_t index = 0) { return find_immediate_impl(value, index); }

        //
        bool is_child_of(const tree_node &other) const { for (auto *p = m_parent; p; p = p->parent()) if (p == &other) return true; return false; }
        bool is_parent_of(const tree_node &other) const { return other.is_child_of(*this); }

        //
        bool deserialize(uint8_t const* buffer, size_t const& size);
        bool deserialize(std::vector<uint8_t> const& buffer);
        bool deserialize(std::span<uint8_t> const& buffer);
        bool deserialize(reader_t &reader);

        //
        bool deserialize(bytes_t const& buffer);
        bool deserialize(bytespan_t const& buffer);

        //
        bool serialize(writer_t &buffer) const;

        //
        void dump(const size_t &indentation = 0, std::ostream &stream = std::cout) const { std::function<void(const tree_node &, size_t)> rec = [&](const tree_node &node, const size_t &indent) { stream << std::setw(static_cast<int>(indent) + 1) << node.m_data; for (auto const &child : node.m_children) rec(child, indent + 2); }; rec(*this, indentation); }
    private: //
        template<typename U> tree_node *find_impl(U value, size_t index = 0) const { std::queue<const tree_node *> q; q.push(this); while (!q.empty()) { auto *node = q.front(); q.pop(); if (node->m_data == value && !(index--)) return const_cast<tree_node *>(node); for (auto const &child : node->m_children) q.push(&child); } return nullptr; }
        template<typename U> tree_node *find_immediate_impl(U value, size_t index = 0) const { for (auto const &child : m_children) if (child.m_tag == value && !(index--)) return const_cast<tree_node *>(&child); return nullptr; }
        tree_node *m_parent = 0; T m_data = {}; std::list<tree_node> m_children = {};
    };
}

#endif /* D5260195_61AB_4BF6_AF66_AFA17C759B97 */
