/**
 * @file interpreter.hpp
 * @brief Simulation engine and interpreter.
 */

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "core.hpp"
#include "rdesc.hpp"
#include "grammar.hpp"

#include <rdesc/rdesc.h>

#include <map>
#include <memory>
#include <ostream>
#include <set>

class Lex;


class Simulation;

/** @brief Interprets concrete syntax into simulation objects. */
class Interpreter {
public:
    Interpreter(Rdesc &&rdesc_)
        : rdesc { std::move(rdesc_) }
        { rdesc.start(START_SYM); };

    enum rdesc_result pump(struct rdesc_cfg_token tk);

    void interpret_lut(struct rdesc_node &);
    void interpret_wire(struct rdesc_node &);
    void interpret_unit(struct rdesc_node &);

    Table interpret_table(struct rdesc_node &);
    std::unique_ptr<TableValue> interpret_table_value(struct rdesc_node &);

    std::ostream &dump(std::ostream &os, const Lex &lex) const;

private:
    friend Simulation;

    std::map<LutId, Lut> luts;
    std::map<WireId, Wire> wires;
    std::map<UnitId, Unit> units;

    static const enum nt START_SYM = NT_STMT;

    Rdesc rdesc;
};

/** @brief core simulation engine. */
class Simulation {
public:
    Simulation(Interpreter &intr)
        : luts { intr.luts }, wires { intr.wires }, units { intr.units } {}

    void set_wire_state(WireId id, bool state);

    void advance();

    void stabilize();

private:
    const std::map<LutId, Lut> &luts;
    std::map<WireId, Wire> &wires;
    const std::map<UnitId, Unit> &units;

    std::set<WireId> changed_wires;
};


#endif
