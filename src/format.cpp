#include "../include/core.hpp"
#include "../include/interpreter.hpp"

#include <ostream>
#include <cstddef>

using std::ostream;


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

    os << ")" << lut.table << ";";

    return os;
}

ostream &operator<<(ostream &os, const Wire &wire) {
    os << "wire w" << wire.id << " = " << (wire.state ? "1" : "0");

    os << wire.table << ";";

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

    os << ")" << unit.table << ";";

    return os;
}

ostream &operator<<(ostream &os, const Table &table) {
    if (table.table.size() == 0)
        return os;

    os << "\n{\n";

    size_t i = 0;
    for (const auto &entry : table.table) {
        os << "    p" << entry.first << ": " << *entry.second;
        os << (i == table.table.size() - 1 ? "\n" : ",\n");
        i++;
    }

    os << "}";

    return os;
}

ostream &operator<<(ostream &os, const Interpreter &intr) {
    for (auto &it : intr.luts)
        os << it.second << "\n";

    if (intr.luts.size() && intr.wires.size())
        os << "\n";

    for (auto &it : intr.wires)
        os << it.second << "\n";

    if (intr.wires.size() && intr.units.size())
        os << "\n";

    for (auto &it : intr.units)
        os << it.second << "\n";

    return os;
}
