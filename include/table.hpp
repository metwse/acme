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
#include <utility>
#include <vector>
#include <ostream>


typedef size_t TableKeyId;

/** @brief Semantic information for table value. */
class TableValue {
public:
    virtual ~TableValue() = default;

    virtual std::ostream &dump(std::ostream &os, const Lex &lex) const = 0;
};

/** @brief Decimal table value. */
class TVNum : public TableValue {
public:
    TVNum(struct rdesc_node &);

    virtual ~TVNum() = default;

    std::ostream &dump(std::ostream &os,
                       [[maybe_unused]] const Lex &lex) const override
        { os << decimal; return os; }

    size_t decimal;
};

/**
 * @brief Point semantic information, which can either be a position of an unit
 * (ident) or (x, y).
 */
class TVPoint : public TableValue {
public:
    virtual ~TVPoint() = default;
};

/** @brief Position aliasing a port of an unit. */
class TVPointIdent : public TVPoint {
public:
    TVPointIdent(struct rdesc_node &);

    virtual ~TVPointIdent() = default;

    std::ostream &dump(std::ostream &os, const Lex &lex) const override
        { os << lex.ident_name(id) << " /*i" << id << "*/"; return os; }

    size_t id;
};

/** @brief Numeric position. */
class TVPointNum : public TVPoint {
public:
    TVPointNum(struct rdesc_node &);

    virtual ~TVPointNum() = default;

    std::ostream &dump(std::ostream &os,
                       [[maybe_unused]] const Lex &lex) const override
        { os << "(" << x << ", " << y << ")"; return os; }

    size_t x;
    size_t y;
};

/** @brief Path of an wire or shape of a lookup table. */
class TVPath : public TableValue {
public:
    TVPath(struct rdesc_node &);

    virtual ~TVPath() = default;

    std::ostream &dump(std::ostream &os, const Lex &lex) const override {
        os << "[";

        size_t j = 0;
        for (auto &path : paths) {
            size_t i = 0;
            for (auto &point_ : path) {
                TableValue *point = point_.get();
                point->dump(os, lex);

                if (i != path.size() - 1)
                    os << ", ";

                i++;
            }

            if (j != paths.size() - 1)
                os << "; ";

            j++;
        }

        os << "]";

        return os;
    }

    std::vector<std::vector<std::unique_ptr<TVPoint>>> paths;
};

/** @brief Extra information table for statements. */
class Table {
public:
    Table(std::map<TableKeyId, std::unique_ptr<TableValue>>  table_)
        : table { std::move(table_) } {}

    Table(Table &&other)
        : table { std::move(other.table)} {}

    std::ostream &dump(std::ostream &os, const Lex &lex) const;

private:
    std::map<TableKeyId, std::unique_ptr<TableValue>> table;
};


#endif
