#include <tinyfsm.hpp>

#include "elevator.hpp"
#include "fsmlist.hpp"


class Idle; // forward declaration


// ----------------------------------------------------------------------------
// Transition functions
//

static void CallMaintenance() {
  std::cout << "*** calling maintenance ***" << std::endl;
}

static void CallFirefighters() {
  std::cout << "*** calling firefighters ***" << std::endl;
}


// ----------------------------------------------------------------------------
// State: Panic
// 状态机状态 Panic

class Panic
: public Elevator
{
  void entry() override {
    send_event(MotorStop());
  }
};


// ----------------------------------------------------------------------------
// State: Moving
// 状态机状态 Moving

class Moving
: public Elevator
{
  void react(FloorSensor const & e) override {
    int floor_expected = current_floor + Motor::getDirection();
    if(floor_expected != e.floor)
    {
      std::cout << "Floor sensor defect (expected " << floor_expected << ", got " << e.floor << ")" << std::endl;
      transit<Panic>(CallMaintenance);
    }
    else
    {
      std::cout << "Reached floor " << e.floor << std::endl;
      current_floor = e.floor;
      if(e.floor == dest_floor)
        transit<Idle>();
    }
  };
};


// ----------------------------------------------------------------------------
// State: Idle
// 状态机状态 Idle

class Idle
: public Elevator
{
  void entry() override {
    // MotorStop是一个​​类型（type）​​，它是在 motor.hpp中定义的一个类（继承自 tinyfsm::Event）。
    // MotorStop()是创建这个类型的一个​​临时对象（匿名对象）​​。这里的 ()是调用 MotorStop类的默认构造函数。
    // send_event函数的参数是一个 const &（常量引用），它需要绑定到一个实际的对象上。所以不能传递一个类型，而必须传递一个该类型的实例。
    // 简单来说：
    // send_event(MotorStop);-> ​​错误​​。这是在传递一个类型名，编译器会报错。
    // send_event(MotorStop());-> 正确。这是在创建一个 MotorStop类的临时对象并将其传递给函数。
    // 等价于：
    // MotorStop event;
    // send_event(event);


    send_event(MotorStop());
  }

  void react(Call const & e) override {
    dest_floor = e.floor;

    if(dest_floor == current_floor)
      return;

    /* lambda function used for transition action */
    auto action = [] { 
      if(dest_floor > current_floor)
        send_event(MotorUp());
      else if(dest_floor < current_floor)
        send_event(MotorDown());
    };

    transit<Moving>(action);
  };
};


// ----------------------------------------------------------------------------
// Base state: default implementations
// 

void Elevator::react(Call const &) {
  std::cout << "Call event ignored" << std::endl;
}

void Elevator::react(FloorSensor const &) {
  std::cout << "FloorSensor event ignored" << std::endl;
}

void Elevator::react(Alarm const &) {
  transit<Panic>(CallFirefighters);
}

int Elevator::current_floor = Elevator::initial_floor;
int Elevator::dest_floor    = Elevator::initial_floor;


// ----------------------------------------------------------------------------
// Initial state definition
//
FSM_INITIAL_STATE(Elevator, Idle)
