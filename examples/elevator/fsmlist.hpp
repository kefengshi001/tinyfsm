#ifndef FSMLIST_HPP_INCLUDED
#define FSMLIST_HPP_INCLUDED

#include <tinyfsm.hpp>

#include "elevator.hpp"
#include "motor.hpp"

using fsm_list = tinyfsm::FsmList<Motor, Elevator>;

/** dispatch event to both "Motor" and "Elevator" */
template<typename E>
void send_event(E const & event)
{
  // 依赖名称的模板消歧义
  // 对一个依赖类型的模板成员函数进行调用 时（即 fsm_list 是模板实例生成的类型），C++ 要求必须写：
  // fsm_list::template dispatch<E>(event);
  // 如果写成 fsm_list::dispatch<E>(event); 编译器会报错，提示找不到 dispatch 成员函数。
  // 编译器不知道 dispatch 是模板还是变量。所以要手动标注 template
  // 访问模板生成类型里的模板成员时，需要加 template 关键字


  fsm_list::template dispatch<E>(event);
}


#endif
