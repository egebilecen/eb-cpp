#ifndef _FSM_H
#define _FSM_H

#include <map>

template <class StateId> class StateMachine;

template <class StateId> class State {
  protected:
    StateId id_;
    StateMachine<StateId> *fsm_;
    std::map<StateId, State<StateId> *> transitions_;

  public:
    explicit State(StateMachine<StateId> *fsm, StateId id)
        : fsm_(fsm), id_(id) {}

    virtual ~State() {}

    inline void add_transition(State<StateId> *state) {
        transitions_[state->get_id()] = state;
    }

    inline State<StateId> *get_transition(StateId id) {
        return transitions_[id];
    }

    inline StateId get_id() { return id_; }

    virtual void on_enter() {}

    virtual void on_update(void *data) {}

    virtual void on_exit() {}
};

template <class StateId> class StateMachine {
  protected:
    std::map<StateId, State<StateId> *> states_;
    State<StateId> *curr_state_;

  public:
    StateMachine() : curr_state_(nullptr) {}

    ~StateMachine() {
		for (const auto& state : states_)
			delete state.second;
	}

    template <class StateType> StateType *add_state() {
        StateType *state = new StateType(this);

        if (states_.count(state->get_id())) {
            delete state;
            return nullptr;
        }

        states_[state->get_id()] = state;
        return state;
    }

    inline State<StateId> *get_state(StateId id) { return states_[id]; }

    inline State<StateId> *get_curr_state() { return curr_state_; }

    inline void update(void *data = nullptr) {
        if (curr_state_)
            curr_state_->on_update(data);
    }

    bool transition_to(StateId id) {
        State<StateId> *state = states_[id];

        if (!state)
            return false;

        if (curr_state_) {
            if (id == curr_state_->get_id()
			|| !curr_state_->get_transition(id))
                return false;

            curr_state_->on_exit();
        }

        curr_state_ = state;
        curr_state_->on_enter();

        return true;
    }
};

#endif
