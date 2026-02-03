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

    std::vector<bool> lookup(std::vector<bool> &);

    LutId id;
    size_t input_size;
    size_t output_size;

private:
    std::vector<bool> lut;
};

class Wire {
public:
    Wire(WireId id_, bool state_)
        : id { id_ }, state { state_ } {}
    WireId id;
    bool state;

    std::set<UnitId> affects {};
};

class Unit {
public:
    Unit(UnitId id_, LutId lut_id_,
         std::vector<WireId> &&input_wires_,
         std::vector<WireId> &&output_wires_)
        : id { id_ }, lut_id { lut_id_ },
          input_wires { std::move(input_wires_) },
          output_wires { std::move(output_wires_) } {}

    UnitId id;
    LutId lut_id;

    std::vector<WireId> input_wires;
    std::vector<WireId> output_wires;
};


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
    std::map<LutId, Lut> luts;
    std::map<WireId, Wire> wires;
    std::map<UnitId, Unit> units;

    Rdesc rdesc;
};

class Simulation {
};


#endif
