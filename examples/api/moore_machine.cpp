#include <tinyfsm.hpp>
#include <iostream>

// ----------------------------------------------------------------------------
// 1. Event Declarations
//
struct Toggle : tinyfsm::Event { };


// ----------------------------------------------------------------------------
// 2. State Machine Base Class Declaration
//派生类把自己作为模板参数传递给基类MooreMachine，从而创建一个特定类型的状态机Switch。
struct Switch : tinyfsm::MooreMachine<Switch>
{
  /* pure virtual reaction (override required in all states) */
  virtual void react(Toggle const &) = 0;
};


// ----------------------------------------------------------------------------
// 3. State Declarations
//
struct Off; // forward declaration

struct On : Switch
{
  void entry() override { std::cout << "* Closing ciruit (light goes ON)" << std::endl; };
  void react(Toggle const &) override { transit<Off>(); };
};

struct Off : Switch
{
  void entry() override { std::cout << "* Opening ciruit (light goes OFF)" << std::endl; };
  void react(Toggle const &) override { transit<On>(); };
};

// 通过宏FSM_INITIAL_STATE(Switch, Off)来特化set_initial_state函数，将初始状态设置为Off
FSM_INITIAL_STATE(Switch, Off)


// ----------------------------------------------------------------------------
// Main
//
int main()
{
  Switch::start();

  std::cout << "> You are facing a light switch..." << std::endl;
  while(1)
  {
    char c;
    std::cout << std::endl << "t=Toggle, q=Quit ? ";
    std::cin >> c;
    switch(c) {
    case 't':
      std::cout << "> Toggling switch..." << std::endl;
      Switch::dispatch(Toggle());
      break;
    case 'q':
      return 0;
    default:
      std::cout << "> Invalid input" << std::endl;
    };
  }
}
