#include "../include/core.hpp"
#include "../include/grammar.hpp"
#include "../include/lex.hpp"
#include "../include/table.hpp"
#include "detail.h"

#include "interpreter.ixx"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <stdexcept>

using std::vector, std::map;
using std::unique_ptr, std::make_unique;
using std::string;
using std::piecewise_construct, std::forward_as_tuple;


static auto get_rrr_ident_id(struct rdesc_node *ls) {
    vector<size_t> res;

    traverse_rrr_list(ls, [&](auto *entry) {
        res.push_back(get_seminfo<IdentInfo>(entry)->id);
    });

    return res;
}


TVNum::TVNum(struct rdesc_node &num)
    : decimal { get_seminfo<NumInfo>(&num)->decimal() } {}

TVPointIdent::TVPointIdent(struct rdesc_node &point)
    : id { get_seminfo<IdentInfo>(point.nt.children[0])->id } {}

TVPointNum::TVPointNum(struct rdesc_node &num)
    : x { get_seminfo<NumInfo>(num.nt.children[1])->decimal() },
      y { get_seminfo<NumInfo>(num.nt.children[3])->decimal() } {}

static unique_ptr<TVPoint> interpret_tvpoint(struct rdesc_node &point) {
    switch (point.nt.variant) {
    case 0: /* ident */
        return make_unique<TVPointIdent>(point);
    case 1: /* num, num */
        return make_unique<TVPointNum>(point);
    default:
        unreachable();  // GCOVR_EXCL_LINE
    }
}

TVPath::TVPath(struct rdesc_node &path_ls) {
    paths.push_back({});
    size_t i = 0;

    traverse_rrr_list(path_ls.nt.children[1], [&](auto *entry, auto *delim) {
        paths[i].push_back(interpret_tvpoint(*entry));

        if (delim && delim->nt.children[0]->tk.id == TK_SEMI) {
            paths.push_back({});
            i++;
        }
    });
}

unique_ptr<TableValue> Interpreter::interpret_table_value(struct rdesc_node &tv) {
    struct rdesc_node &child = *tv.nt.children[0];
    switch (tv.nt.variant) {
    case 0: /* num */
        return make_unique<TVNum>(child);
    case 1: /* tv_point */
        return interpret_tvpoint(child);
    case 2: /* tv_path */
        return make_unique<TVPath>(child);
    default:
        unreachable();  // GCOVR_EXCL_LINE
    }
}

Table Interpreter::interpret_table(struct rdesc_node &n) {
    map<TableKeyId, unique_ptr<TableValue>> table;

    if (n.nt.child_count == 0)
        return Table { std::move(table) };

    auto ls = n.nt.children[0]->nt.children[1];

    traverse_rrr_list(ls, [&](auto *entry) {
        TableKeyId key = get_seminfo<IdentInfo>(entry->nt.children[0])->id;

        table.emplace(
            piecewise_construct,
            forward_as_tuple(key),
            forward_as_tuple(interpret_table_value(*entry->nt.children[2]))
        );
    });


    return Table { std::move(table) };
}

static void parse_lut_num_info(vector<bool> &table,
                               size_t input_variant_count,
                               const NumInfo &info) {
    if (info.base == 10) {
        uintmax_t value = info.decimal();
        for (size_t i = 0; i < input_variant_count; i++)
            table.push_back((value >> i) & 1);
    } else {
        const string &num_str = info.num;
        size_t bit_index = 0;

        for (auto it = num_str.rbegin();
             it != num_str.rend() && bit_index < input_variant_count;
             ++it) {
            char digit = *it;
            uintmax_t digit_value = 0;

            if ('0' <= digit && digit <= '9')
                digit_value = digit - '0';
            else if ('a' <= digit && digit <= 'f')
                digit_value = digit - 'a' + 10;
            else if ('A' <= digit && digit <= 'F')
                digit_value = digit - 'A' + 10;
            else
                unreachable();  // GCOVR_EXCL_LINE

            int bits_per_digit = (info.base == 2) ? 1 : (info.base == 8) ? 3 : 4;
            for (int b = 0;
                 b < bits_per_digit && bit_index < input_variant_count;
                 b++, bit_index++)
                table.push_back((digit_value >> b) & 1);
        }

        for (; bit_index < input_variant_count; bit_index++)
            table.push_back(false);
    }
}

void Interpreter::interpret_lut(struct rdesc_node &lut) {
    auto nt = lut.nt;

    size_t input_size = get_seminfo<NumInfo>(nt.children[2])->decimal();
    size_t output_size = get_seminfo<NumInfo>(nt.children[4])->decimal();
    LutId id = get_seminfo<IdentInfo>(nt.children[6])->id;

    auto lookup_table_ = get_rrr_seminfo<NumInfo>(nt.children[9]);
    /* end of serialization */

    if (lookup_table_.size() != output_size)
        throw std::length_error("lookup table does not match output size "
                                "with lut");
    /* end of validation */

    vector<bool> lookup_table;
    lookup_table.reserve((1 << input_size) * output_size);

    for (auto &output_values : lookup_table_)
        parse_lut_num_info(lookup_table, 1 << input_size, *output_values);

    luts.emplace(
        piecewise_construct,
        forward_as_tuple(id),
        forward_as_tuple(
            interpret_table(*nt.children[11]),
            id, input_size, output_size,
            std::move(lookup_table)
        )
    );
};

void Interpreter::interpret_wire(struct rdesc_node &wire) {
    auto nt = wire.nt;

    WireId id = get_seminfo<IdentInfo>(nt.children[1])->id;
    bool state = get_seminfo<NumInfo>(nt.children[3])->decimal() != 0;
    /* end of serialization */
    /* end of validation */

    wires.emplace(
        piecewise_construct,
        forward_as_tuple(id),
        forward_as_tuple(
            interpret_table(*nt.children[4]), id, state
        )
    );
};

void Interpreter::interpret_unit(struct rdesc_node &unit) {
    auto nt = unit.nt;

    LutId lut_id = get_seminfo<IdentInfo>(nt.children[2])->id;
    LutId id = get_seminfo<IdentInfo>(nt.children[4])->id;

    auto input_wires = get_rrr_ident_id(nt.children[7]);
    auto output_wires = get_rrr_ident_id(nt.children[11]);
    /* end of serialization */

    auto validate_input_wires = [this](const auto &wire_ids) {
        for (auto &id : wire_ids)
            if (!wires.contains(id))
                throw std::invalid_argument("unknown wire");
    };

    validate_input_wires(input_wires);
    validate_input_wires(output_wires);

    if (!luts.contains(lut_id))
        throw std::invalid_argument("unknown lut");

    auto &lut = luts.at(lut_id);

    if (lut.input_size != input_wires.size())
        throw std::length_error("invalid input wire size");
    if (lut.output_size != output_wires.size())
        throw std::length_error("invalid output wire size");
    /* end of validation */

    for (auto input_wire : input_wires)
        wires.at(input_wire).affects.insert(id);

    units.emplace(
        piecewise_construct,
        forward_as_tuple(id),
        forward_as_tuple(
            interpret_table(*nt.children[13]),
            id, lut_id,
            std::move(input_wires),
            std::move(output_wires)
        )
    );
};

enum rdesc_result Interpreter::pump(struct rdesc_cfg_token tk) {
    struct rdesc_node *cst = NULL;

    auto res = rdesc.pump(&cst, &tk);

    switch (res) {
    case RDESC_CONTINUE:
        return RDESC_CONTINUE;
    case RDESC_NOMATCH:
        rdesc.reset(tk_destroyer);
        rdesc.start(START_SYM);
        return RDESC_NOMATCH;
    default:
        break;
    }

    struct rdesc_node &stmt = *cst->nt.children[0];

    try {
        switch (stmt.nt.id) {
        case NT_LUT:
            interpret_lut(stmt);
            break;
        case NT_WIRE:
            interpret_wire(stmt);
            break;
        case NT_UNIT:
            interpret_unit(stmt);
            break;
        }
    } catch (...) {
        rdesc_node_destroy(cst, NULL);
        rdesc.start(START_SYM);
        throw;
    }

    rdesc_node_destroy(cst, NULL);
    rdesc.start(START_SYM);
    return RDESC_READY;
}
