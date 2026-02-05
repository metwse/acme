/**
 * @file core.hpp
 * @brief Core simulation engine.
 */

#ifndef CORE_HPP
#define CORE_HPP


#include "grammar.hpp"
#include "rdesc.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <cstddef>
#include <map>
#include <set>
#include <vector>


typedef size_t LutId;
typedef size_t UnitId;
typedef size_t WireId;


class Lut {
public:
    Lut(LutId id_, size_t input_size_, size_t output_size_,
        std::vector<bool> &&lut_)
        : id { id_ }, input_size { input_size_ }, output_size { output_size_ },
          lut { std::move(lut_) } {}

    std::vector<bool> lookup(const std::vector<bool> &) const;

    size_t input_variant_count() const
        { return (1 << input_size); }

    const LutId id;
    const size_t input_size;
    const size_t output_size;

private:
    friend std::ostream &operator<<(std::ostream &, const Lut &);

    std::vector<bool> lut;
};

class Wire {
public:
    Wire(WireId id_, bool state_)
        : id { id_ }, state { state_ } {}
    WireId id;
    bool state;

    std::set<UnitId> affects {};

private:
    friend std::ostream &operator<<(std::ostream &, const Wire &);
};

class Unit {
public:
    Unit(UnitId id_, LutId lut_id_,
         std::vector<WireId> &&input_wires_,
         std::vector<WireId> &&output_wires_)
        : id { id_ }, lut_id { lut_id_ },
          input_wires { std::move(input_wires_) },
          output_wires { std::move(output_wires_) } {}

    const UnitId id;
    const LutId lut_id;

    const std::vector<WireId> input_wires;
    const std::vector<WireId> output_wires;

private:
    friend std::ostream &operator<<(std::ostream &, const Unit &);
};

class Simulation;

class Interpreter {
public:
    Interpreter(Rdesc &&rdesc_)
        : rdesc { std::move(rdesc_) }
        { rdesc.start(NT_STMT); };

    enum rdesc_result pump(struct rdesc_cfg_token tk);

    void interpret_lut(struct rdesc_node &);
    void interpret_wire(struct rdesc_node &);
    void interpret_unit(struct rdesc_node &);

private:
    friend std::ostream &operator<<(std::ostream &, const Interpreter &);
    friend Simulation;

    std::map<LutId, Lut> luts;
    std::map<WireId, Wire> wires;
    std::map<UnitId, Unit> units;

    Rdesc rdesc;
};

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
