#include "../include/core.hpp"
#include "../include/grammar.hpp"
#include "../include/lex.hpp"
#include "detail.h"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <stdexcept>

using std::vector;
using std::unique_ptr;
using std::piecewise_construct, std::forward_as_tuple;
using std::ostream;
using std::string;


template<typename T>
static inline auto get_seminfo(struct rdesc_node *tk) {
    return unique_ptr<T>(static_cast<T *>(tk->tk.seminfo));
}

template<typename T>
static auto get_rrr_seminfo(struct rdesc_node *ls) {
    vector<unique_ptr<T>> res;

    do {
        res.push_back(get_seminfo<T>(ls->nt.children[0]));

        ls = ls->nt.children[1];
    } while (ls->nt.child_count && (ls = ls->nt.children[1]));

    return res;
}  // GCOVR_EXCL_LINE

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
            uintmax_t digit_value;

            if ('0' <= digit && digit <= '9')  // GCOVR_EXCL_LINE: due to gcovr false-negative
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

    auto table_ = get_rrr_seminfo<NumInfo>(nt.children[9]);
    /* end of serialization */

    if (table_.size() != output_size)
        throw std::length_error("lookup table does not match output size "
                                "with lut");
    /* end of validation */

    vector<bool> table;
    table.reserve((1 << input_size) * output_size);

    for (auto &output_values : table_)
        parse_lut_num_info(table, 1 << input_size, *output_values);

    luts.emplace(piecewise_construct,
                 forward_as_tuple(id),
                 forward_as_tuple(id, input_size, output_size,
                                  std::move(table)));
};

void Interpreter::interpret_wire(struct rdesc_node &wire) {
    auto nt = wire.nt;

    WireId id = get_seminfo<IdentInfo>(nt.children[1])->id;
    bool state = get_seminfo<NumInfo>(nt.children[3])->decimal() != 0;
    /* end of serialization */
    /* end of validation */

    wires.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, state));
};

void Interpreter::interpret_unit(struct rdesc_node &unit) {
    auto nt = unit.nt;

    LutId lut_id = get_seminfo<IdentInfo>(nt.children[2])->id;
    LutId id = get_seminfo<IdentInfo>(nt.children[4])->id;

    auto input_wires_ = get_rrr_seminfo<IdentInfo>(nt.children[7]);
    auto output_wires_ = get_rrr_seminfo<IdentInfo>(nt.children[11]);

    auto extract_wire_ids = [](const auto &info_list) {
        vector<WireId> ids;
        ids.reserve(info_list.size());
        for (const auto &info : info_list)
            ids.push_back(info->id);
        return ids;
    };  // GCOVR_EXCL_LINE

    vector<WireId> input_wires = extract_wire_ids(input_wires_);
    vector<WireId> output_wires = extract_wire_ids(output_wires_);
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

    auto lut = luts.at(lut_id);

    if (lut.input_size != input_wires.size())
        throw std::length_error("invalid input wire size");
    if (lut.output_size != output_wires.size())
        throw std::length_error("invalid output wire size");
    /* end of validation */

    for (auto input_wire : input_wires)
        wires.at(input_wire).affects.insert(id);

    units.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, lut_id,
                                   std::move(input_wires),
                                   std::move(output_wires)));
};

enum rdesc_result Interpreter::pump(struct rdesc_cfg_token tk) {
    struct rdesc_node *cst = NULL;

    auto res = rdesc.pump(&cst, &tk);

    switch (res) {
    case RDESC_CONTINUE:
        return RDESC_CONTINUE;
    case RDESC_NOMATCH:
        rdesc.reset(tk_destroyer);
        rdesc.start(NT_STMT);
        return RDESC_NOMATCH;
    default:
        break;
    }

    struct rdesc_node &stmt = *cst->nt.children[0];

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

    rdesc_node_destroy(cst, NULL);
    rdesc.start(NT_STMT);
    return RDESC_READY;
}
ostream &operator<<(ostream &os, const Lut &lut) {
    os << "lut<" << lut.input_size << ", " << lut.output_size << "> l"
        << lut.id << " = (0b";

    size_t input_variant_count = lut.input_variant_count();
    for (size_t i = 0; i < lut.lut.size(); i++) {
        if (i > 0 && i % input_variant_count == 0)
            os << ", 0b";

        os << lut.lut[
            (i / input_variant_count + 1) * input_variant_count -
            (i % input_variant_count) - 1
            ];
    }

    os << ");";

    return os;
}

ostream &operator<<(ostream &os, const Wire &wire) {
    os << "wire w" << wire.id << " = " << (wire.state ? "1;" : "0;");

    return os;
}

ostream &operator<<(ostream &os, const Unit &unit) {
    os << "unit<l" << unit.lut_id << "> u" << unit.id << " = (";

    for (size_t i = 0; i < unit.input_wires.size(); i++) {
        if (i > 0)
            os << ", ";

        os << "w" << unit.input_wires[i];
    }

    os << ") -> (";

    for (size_t i = 0; i < unit.output_wires.size(); i++) {
        if (i > 0)
            os << ", ";

        os << "w" << unit.output_wires[i];
    }

    os << ");";

    return os;
}

ostream &operator<<(ostream &os, const Interpreter &intr) {
    for (auto &it : intr.luts)
        os << it.second << "\n";

    os << "\n";

    for (auto &it : intr.wires)
        os << it.second << "\n";

    os << "\n";

    for (auto &it : intr.units)
        os << it.second << "\n";

    return os;
}
