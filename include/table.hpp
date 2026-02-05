/**
 * @file table.hpp
 * @brief Additional information table for statements, which exclusively used
 * in visualizer.
 */

#ifndef TABLE_HPP
#define TABLE_HPP


#include <map>
#include <memory>


typedef size_t TableKeyId;


class TableValue {
public:
};


class Table {
public:
    Table(struct rdesc_node *);

private:
    std::map<TableKeyId, std::unique_ptr<TableValue>> table;
};


#endif
