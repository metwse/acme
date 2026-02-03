#include "core.hpp"
#include "grammar.hpp"
#include "lex.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <tuple>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>

using std::vector;
using std::unique_ptr;
using std::piecewise_construct, std::forward_as_tuple;


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

    vector<bool> table;  // TODO: lut parsing

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
    };

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
        throw std::invalid_argument("invalid input wire size");
    if (lut.output_size != output_wires.size())
        throw std::invalid_argument("invalid output wire size");
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
