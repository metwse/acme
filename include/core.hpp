/**
 * @file core.hpp
 * @brief Core simulation elements.
 */

#ifndef CORE_HPP
#define CORE_HPP


#include "table.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <cstddef>
#include <set>
#include <utility>
#include <vector>

class Lex;


/** @brief New-type pattern for lookup table identifiers. */
typedef size_t LutId;
/** @brief New-type pattern for unit identifiers. */
typedef size_t UnitId;
/** @brief New-type pattern for wire identifiers. */
typedef size_t WireId;

/** @brief Lookup table component. */
class Lut {
public:
    Lut(Table table, LutId id_, size_t input_size_, size_t output_size_,
        std::vector<bool> &&lut_)
        : table { std::move(table) },
          id { id_ }, input_size { input_size_ }, output_size { output_size_ },
          lut { std::move(lut_) } {}

    std::vector<bool> lookup(const std::vector<bool> &) const;

    std::ostream &dump(std::ostream &os, const Lex &lex) const;

    size_t input_variant_count() const
        { return (1 << input_size); }

    const Table table;

    const LutId id;
    const size_t input_size;
    const size_t output_size;

private:
    std::vector<bool> lut;
};

/** @brief Wire representing pyhsical connections. */
class Wire {
public:
    Wire(Table table, WireId id_, bool state_)
        : table { std::move(table) },
          id { id_ }, state { state_ } {}

    std::ostream &dump(std::ostream &os, const Lex &lex) const;

    const Table table;

    const WireId id;
    bool state;

    std::set<UnitId> affects {};
};

/** @brief Pyhsical logic elements in the simulation. */
class Unit {
public:
    Unit(Table table, UnitId id_, LutId lut_id_,
         std::vector<WireId> &&input_wires_,
         std::vector<WireId> &&output_wires_)
        : table { std::move(table) },
          id { id_ }, lut_id { lut_id_ },
          input_wires { std::move(input_wires_) },
          output_wires { std::move(output_wires_) } {}

    std::ostream &dump(std::ostream &os, const Lex &lex) const;

    const Table table;

    const UnitId id;
    const LutId lut_id;

    const std::vector<WireId> input_wires;
    const std::vector<WireId> output_wires;
};


#endif
