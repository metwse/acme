/**
 * @file table.hpp
 * @brief Additional information table for statements, which exclusively used
 * in visualizer.
 */

#ifndef TABLE_HPP
#define TABLE_HPP


#include "lex.hpp"

#include <map>
#include <memory>


typedef size_t TableKeyId;


class Table {
public:
    Table(struct rdesc_node *);

private:
    std::map<TableKeyId, std::unique_ptr<TableValueInfo>> table;
};


#endif
