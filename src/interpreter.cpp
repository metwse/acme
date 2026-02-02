#include "core.hpp"
#include "grammar.hpp"
#include "lex.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <tuple>
#include <utility>
#include <vector>

using std::vector;
using std::unique_ptr;
using std::piecewise_construct, std::forward_as_tuple;


#define IDENT_ID(TK) reinterpret_cast<IdentInfo *>(TK->tk.seminfo)->id
#define NUM_INFO(TK) reinterpret_cast<NumInfo *>(TK->tk.seminfo)


static inline auto collect_rrr(struct rdesc_node *ls) {
    vector<struct rdesc_node *> res;

    while (true) {
        res.push_back(ls->nt.children[0]);

        ls = ls->nt.children[1];
        if (!ls->nt.child_count)
            break;

        ls = ls->nt.children[1];
    }

    return res;
}

void Interpreter::interpret_lut(struct rdesc_node *lut) {
    auto nt = lut->nt;

    size_t i = static_cast<size_t>(IDENT_ID(nt.children[2]));
    size_t o = static_cast<size_t>(IDENT_ID(nt.children[4]));
    LutId id = IDENT_ID(nt.children[6]);

    /// TODO: lookup table parsing
    vector<unique_ptr<Bitvec>> bv;
    collect_rrr(nt.children[9]);

    luts.emplace(piecewise_construct,
                 forward_as_tuple(id),
                 forward_as_tuple(id, i, o, std::move(bv)));

    rdesc_node_destroy(lut, tk_destroyer);
};

void Interpreter::interpret_wire(struct rdesc_node *wire) {
    auto nt = wire->nt;

    WireId id = IDENT_ID(nt.children[1]);
    bool state = NUM_INFO(nt.children[1])->decimal() != 0;

    wires.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, state));

    rdesc_node_destroy(wire, tk_destroyer);
};

void Interpreter::interpret_unit(struct rdesc_node *unit) {
    auto nt = unit->nt;

    LutId lut_id = IDENT_ID(nt.children[2]);
    LutId id = IDENT_ID(nt.children[4]);

    /// TODO: input/output parsing
    vector<WireId> i;
    collect_rrr(nt.children[7]);

    vector<WireId> o;
    collect_rrr(nt.children[11]);

    units.emplace(piecewise_construct,
                  forward_as_tuple(id),
                  forward_as_tuple(id, lut_id, i, o));

    rdesc_node_destroy(unit, tk_destroyer);
};

bool Interpreter::pump(struct rdesc_cfg_token tk) {
    struct rdesc_node *out = NULL;

    auto res = rdesc.pump(&out, &tk);

    switch (res) {
    case RDESC_CONTINUE:
        return true;
    case RDESC_NOMATCH:
        rdesc.reset(tk_destroyer);
        return false;
    default:
        break;
    }

    auto decl = out->nt.children[0];

    switch (out->nt.id) {
    case NT_LUT:
        interpret_lut(decl);
        break;
    case NT_WIRE:
        interpret_wire(decl);
        break;
    case NT_UNIT:
        interpret_lut(decl);
        break;
    }

    rdesc_node_destroy(out, NULL);
    return true;
}
