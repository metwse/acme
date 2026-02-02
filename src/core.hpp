/**
 * @file core.hpp
 * @brief Core simulation engine.
 */

#ifndef CORE_HPP
#define CORE_HPP


#include "rdesc.hpp"

#include <cstddef>
#include <map>
#include <rdesc/cfg.h>
#include <set>
#include <memory>
#include <vector>


typedef size_t LutId;
typedef size_t UnitId;
typedef size_t WireId;


class Bitvec {
public:
    virtual bool get(size_t) = 0;
};

class Lut {
public:
    Lut(LutId id_, size_t i_, size_t o_, auto lut_)
        : id { id_ }, i { i_ }, o { o_ }, lut { std::move(lut_) } {}

    std::vector<bool> lookup(std::vector<bool>);

    LutId id;
    size_t i;
    size_t o;

private:
    std::vector<std::unique_ptr<Bitvec>> lut;
};

class Wire {
public:
    Wire(WireId id_, bool state_, auto affects_)
        : id { id_ }, state { state_ }, affects { std::move(affects_) } {}

    WireId id;
    bool state;
    std::set<UnitId> affects;
};

class Unit {
public:
    Unit(UnitId id_, LutId lut_id_, auto i_, auto o_)
        : id { id_ }, lut_id { lut_id_ }, i { i_ }, o { o_ } {}

    UnitId id;
    LutId lut_id;
    std::vector<WireId> i;
    std::vector<WireId> o;
};


class Interpreter {
public:
    Interpreter(Rdesc rdesc_)
        : rdesc { std::move(rdesc_) } {};

    bool pump(struct rdesc_cfg_token tk);

private:
    std::map<UnitId, Lut> luts;
    std::map<WireId, Wire> wires;
    std::map<UnitId, Unit> units;

    Rdesc rdesc;
};

class Simulation {
};


#endif
