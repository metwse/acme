#include "../include/core.hpp"
#include "../include/interpreter.hpp"
#include "../include/lex.hpp"

#include <ostream>
#include <cstddef>

using std::ostream;


ostream &Lut::dump(ostream &os, const Lex &lex) const {
    os << "lut<" << input_size << ", " << output_size << "> " <<
        lex.ident_name(id)<< " /*l" << id << "*/ = (0b";

    for (size_t i = 0; i < lut.size(); i++) {
        if (i > 0 && i % input_variant_count() == 0)
            os << ", 0b";

        os << lut[
            (i / input_variant_count() + 1) * input_variant_count() -
            (i % input_variant_count()) - 1
        ];
    }

    os << ")";
    table.dump(os, lex) << ";";

    return os;
}

ostream &Wire::dump(ostream &os, const Lex &lex) const {
    os << "wire " << lex.ident_name(id) << " /*w" << id << "*/ = " <<
        (state ? "1" : "0");

    table.dump(os, lex) << ";";

    return os;
}

ostream &Unit::dump(ostream &os, const Lex &lex) const {
    os << "unit<" << lex.ident_name(lut_id) << " /*l" << lut_id << "*/> " <<
        lex.ident_name(id) << " /*u" << id << "*/ = (";

    for (size_t i = 0; i < input_wires.size(); i++) {
        if (i > 0)
            os << ", ";

        os << lex.ident_name(input_wires[i] )<< " /*w" <<
            input_wires[i] << "*/";
    }

    os << ") -> (";

    for (size_t i = 0; i < output_wires.size(); i++) {
        if (i > 0)
            os << ", ";

        os << lex.ident_name(output_wires[i] )<< " /*w" <<
            output_wires[i] << "*/";
    }

    os << ")";
    table.dump(os, lex) << ";";

    return os;
}

ostream &Table::dump(ostream &os, const Lex &lex) const {
    if (table.size() == 0)
        return os;

    os << "\n{\n";

    size_t i = 0;
    for (const auto &entry : table) {
        os << "    " << lex.ident_name(entry.first) << " /*p" << entry.first
            << "*/: ";

        entry.second->dump(os, lex) << (i == table.size() - 1 ? "\n" : ",\n");

        i++;
    }

    os << "}";

    return os;
}

ostream &Interpreter::dump(ostream &os, const Lex &lex) const {
    for (auto &it : luts)
        it.second.dump(os, lex) << "\n";

    if (luts.size() && wires.size())
        os << "\n";

    for (auto &it : wires)
        it.second.dump(os, lex) << "\n";

    if (wires.size() && units.size())
        os << "\n";

    for (auto &it : units)
        it.second.dump(os, lex) << "\n";

    return os;
}
