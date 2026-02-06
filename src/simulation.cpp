#include "../include/interpreter.hpp"
#include "../include/core.hpp"

#include <set>
#include <vector>

using std::set, std::vector;


vector<bool> Lut::lookup(const vector<bool> &inputs) const {
    vector<bool> res;
    res.reserve(output_size);

    size_t output_index = 0;
    for (size_t i = 0; i < inputs.size(); i++)
        output_index += (inputs[i]) << i;

    size_t i = 0;
    for (size_t offset = 0;
         offset < lut.size();
         offset += input_variant_count()) {
        res[i++] = lut[offset + output_index];
    }

    return res;
};  // GCOVR_EXCL_LINE

void Simulation::set_wire_state(WireId id, bool state) {
    bool &current_state = wires.at(id).state;

    if (current_state != state) {
        current_state = state;
        changed_wires.insert(id);
    }
}

void Simulation::advance() {
    set<WireId> affected_wires;

    set<WireId> changed_wires_ = std::move(changed_wires);
    changed_wires.clear();

    for (WireId wire_id : changed_wires_) {
        for (UnitId unit_id : wires.at(wire_id).affects) {
            const Unit &unit = units.at(unit_id);
            const Lut &lut = luts.at(unit.lut_id);

            vector<bool> inputs;
            inputs.reserve(lut.input_size);

            for (WireId input_wire_id : unit.input_wires)
                inputs.push_back(wires.at(input_wire_id).state);

            vector<bool> outputs = lut.lookup(inputs);

            size_t i = 0;
            for (WireId output_wire_id : unit.output_wires)
                set_wire_state(output_wire_id, outputs[i++]);
        }
    }
}

void Simulation::stabilize() {
    while (changed_wires.size())
        advance();
}
