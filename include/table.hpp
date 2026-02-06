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


class TableValue {
public:
    virtual ~TableValue() = default;

    virtual std::ostream &print(std::ostream &os) const = 0;

    friend std::ostream &operator<<(std::ostream &stream, const TableValue &tv)
        { return tv.print(stream); }
};

class TVNum : public TableValue {
public:
    TVNum(struct rdesc_node &);

    virtual ~TVNum() = default;

    std::ostream &print(std::ostream &os) const override
        { os << numinfo->num; return os; }

    std::unique_ptr<NumInfo> numinfo;
};

class TVPoint : public TableValue {
public:
    virtual ~TVPoint() = default;
};

class TVPointIdent : public TVPoint {
public:
    TVPointIdent(struct rdesc_node &);

    virtual ~TVPointIdent() = default;

    std::ostream &print(std::ostream &os) const override
        { os << "i" << ident->id; return os; }

    std::unique_ptr<IdentInfo> ident;
};

class TVPointNum : public TVPoint {
public:
    TVPointNum(struct rdesc_node &);

    virtual ~TVPointNum() = default;

    std::ostream &print(std::ostream &os) const override
        { os << "(" << num[0]->num << ", " << num[1]->num << ")"; return os; }

    std::unique_ptr<NumInfo> num[2];
};

class TVPath : public TableValue {
public:
    TVPath(struct rdesc_node &);

    virtual ~TVPath() = default;

    std::ostream &print(std::ostream &os) const override {
        size_t i = 0;
        for (auto &point_ : path) {
            TableValue *point = point_.get();
            os << *point;

            if (i != path.size() - 1)
                os << ", ";

            i++;
        }
        return os;
    }

    std::vector<std::unique_ptr<TVPoint>> path;
};


class Table {
public:
    Table(auto table_)
        : table { std::move(table_) } {}

    Table(Table &&other)
        : table { std::move(other.table)} {}

private:
    friend std::ostream &operator<<(std::ostream &, const Table &);

    std::map<TableKeyId, std::unique_ptr<TableValue>> table;
};


#endif
