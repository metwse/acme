#include "core.hpp"
#include "grammar.hpp"
#include "lex.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <tuple>
#include <utility>
#include <vector>
#include <memory>

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

    size_t i = get_seminfo<IdentInfo>(nt.children[2])->id;
    size_t o = get_seminfo<IdentInfo>(nt.children[4])->id;
    LutId id = get_seminfo<IdentInfo>(nt.children[6])->id;

    /// TODO: lookup table parsing
    vector<unique_ptr<Bitvec>> bv;
    get_rrr_seminfo<NumInfo>(nt.children[9]);

    luts.emplace(piecewise_construct,
                 forward_as_tuple(id),
                 forward_as_tuple(id, i, o, std::move(bv)));
};

void Interpreter::interpret_wire(struct rdesc_node &wire) {
    auto nt = wire.nt;

    WireId id = get_seminfo<IdentInfo>(nt.children[1])->id;
    bool state = get_seminfo<NumInfo>(nt.children[3])->decimal() != 0;

    wires.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, state));
};

void Interpreter::interpret_unit(struct rdesc_node &unit) {
    auto nt = unit.nt;

    LutId lut_id = get_seminfo<IdentInfo>(nt.children[2])->id;
    LutId id = get_seminfo<IdentInfo>(nt.children[4])->id;

    /// TODO: input/output parsing
    vector<WireId> i;
    auto _a = get_rrr_seminfo<IdentInfo>(nt.children[7]);

    vector<WireId> o;
    auto _b = get_rrr_seminfo<IdentInfo>(nt.children[11]);

    units.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, lut_id, i, o));
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
