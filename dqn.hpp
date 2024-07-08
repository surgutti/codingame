#ifndef DQN_HPP
#define DQN_HPP

#include <fdeep/fdeep.hpp>

#include <string>
#include <vector>

struct DQNAgent {

    fdeep::model model;

    DQNAgent(const std::string& model_path) : model(fdeep::load_model(model_path)) { }

    int act(const std::vector<float>& state) const {
        const auto input = fdeep::tensor(fdeep::tensor_shape(state.size()), state);
        const auto result = model.predict({input});
        const auto q_values = result.front().to_vector();
        return std::distance(q_values.begin(), std::max_element(q_values.begin(), q_values.end()));
    }

};

#endif // NN_HPP    