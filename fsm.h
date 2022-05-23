#pragma once

#include<map>
#include<memory>

template <typename T>
class FiniteStateMachine;

template<typename T>
class State
{
protected:
    FiniteStateMachine<T>* m_fsm;
    std::map<T, State<T>*> m_transitionList;
    T m_id;

public:
    explicit State(FiniteStateMachine<T>* fsm, T id)
        : m_fsm(fsm)
        , m_id(id)
    {
    }

    virtual ~State(){}

    virtual void enter()
    {
    }

    virtual void exit()
    {
    }

    virtual void update(void* data=nullptr)
    {
    }

    inline T getID() const
    {
        return m_id;
    }

    bool addTransition(State<T>* state)
    {
        T stateID = state->getID();

        if(m_transitionList.count(stateID) > 0) return false;

        m_transitionList[state->getID()] = state;
        return true;
    }

    inline State<T>* getTransition(T stateID)
    {
        return m_transitionList[stateID];
    }
};

template<typename T>
class FiniteStateMachine
{
public:
    FiniteStateMachine()
        : m_currentState(nullptr)
    {
    }

    ~FiniteStateMachine()
    {
        for(const auto& kv : m_stateList)
            delete kv.second;
    }

    inline void update(void* data=nullptr)
    {
        m_currentState->update(data);
    }

    template<class State_>
    State_* addState()
    {
        State_* state = new State_(this);

        if(m_stateList.count(state->getID()) > 0)
        {
            delete state;
            return nullptr;
        }

        m_stateList[state->getID()] = state;
        return state;
    }

    inline State<T>* getState(T stateId)
    {
        return m_stateList[stateId];
    }

    inline State<T>* getCurrentState()
    {
        return m_currentState;
    }

    bool transitionTo(T stateId)
    {
        State<T>* pStateTo = getState(stateId);
        if(pStateTo == nullptr) return false;

        if(m_currentState != nullptr)
        {
            if(stateId == m_currentState->getID()) return false;
            if(m_currentState->getTransition(pStateTo->getID()) == nullptr) return false;
            m_currentState->exit();
        }

        m_currentState = pStateTo;
        m_currentState->enter();

        return true;
    }

protected:
    std::map<T, State<T>*> m_stateList;
    State<T>* m_currentState;
};

