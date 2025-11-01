/*
 * TinyFSM - Tiny Finite State Machine Processor
 *
 * Copyright (c) 2012-2022 Axel Burri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* ---------------------------------------------------------------------
 * Version: 0.3.3
 *
 * API documentation: see "../doc/50-API.md"
 *
 * The official TinyFSM website is located at:
 * https://digint.ch/tinyfsm/
 *
 * Author:
 * Axel Burri <axel@tty0.ch>
 * ---------------------------------------------------------------------
 */

#ifndef TINYFSM_HPP_INCLUDED
#define TINYFSM_HPP_INCLUDED

#ifndef TINYFSM_NOSTDLIB
#include <type_traits>
#include <iostream>
#endif

// #include <iostream>
// #define DBG(str) do { std::cerr << str << std::endl; } while( false )
// DBG("*** dbg_example *** " << __PRETTY_FUNCTION__);

namespace tinyfsm
{

  // --------------------------------------------------------------------------

  struct Event { };

  // --------------------------------------------------------------------------

#ifdef TINYFSM_NOSTDLIB
  // remove dependency on standard library (silent fail!).
  // useful in conjunction with -nostdlib option, e.g. if your compiler
  // does not provide a standard library.
  // NOTE: this silently disables all static_assert() calls below!
  template<typename F, typename S>
  struct is_same_fsm { static constexpr bool value = true; };
#else
  // check if both fsm and state class share same fsmtype
  /*定义了一个模板结构体，用于检查两个类F和S的fsmtype 类型是否一致。
  F通常用于表示状态机类，表示的是状态机的类型，S表示状态类型，表示的是状态机F中的某个状态的类型。*/

  /*此处struct是 定义一个模板结构体类型 
  is_same_fsm<F,S> 本身是一个类型（type），它继承自 std::is_same<...>。因此它拥有 std::is_same 所提供的成员（例如 ::value、::type 等）。
  */
  template<typename F, typename S>
  struct is_same_fsm : std::is_same< typename F::fsmtype, typename S::fsmtype > { };
#endif

  template<typename S>
  struct _state_instance
  {
    /* 等价于 typedef S value_type;  让 value_type 代表模板参数 S 的类型。*/
    using value_type = S;
    
    /* 等价于 typedef _state_instance<S> type;  定义一个类型别名 type，指向自己；
       让别人能用 _state_instance<S>::type 来指代 _state_instance<S> 自身 */
    using type = _state_instance<S>;  

    /* 只是声明（declaration），告诉编译器 “这个类里有一个静态成员变量”。但它没有定
       义（definition），也就是说编译器还没真正为它分配内存空间*/
    // static 关键字修饰结构体/类 成员变量时,可以不需要实例化就直接访问该成员变量
    static S value;         
  };


  /* 定义一个静态成员变量 value，类型为 _state_instance<S>::value_type，即模板参数 S 的类型。 
     typename 告诉编译器，typename 后面跟的标识符是一个类型，即 _state_instance<S>::value_type。
     对所有模板类型 S，去定义一个 _state_instance<S>::value 静态变量。 */

  /* 静态成员变量 value 的定义：
    1.当value_type = S 始终成立时，也就是说不会修改value_type的类型，那么可以将value定义成如下形式：
      template<typename S>
      S _state_instance<S>::value;  // 定义静态成员变量 value，类型为 S
    2.当value_type 可能被修改时（value_type =  _state_instance<S>），那么就需要使用 value_type 来定义 value：
    template<typename S>
    typename _state_instance<S>::value_type _state_instance<S>::value;  //注意结构体中最好将static S value 修改成 static value_type value; 

    采用方式二来定义鲁棒性更强，至于为什么要加typename关键字，因为编译器不知道_state_instance<S>::value_type是类型还是静态成员变量，所以需要用typename来告诉编译器它是一个类型。
  */

  template<typename S>
  typename _state_instance<S>::value_type _state_instance<S>::value;  

  // --------------------------------------------------------------------------

  template<typename F>
  class Fsm
  {
  public:

    using fsmtype = Fsm<F>;
    using state_ptr_t = F *;

    /*这是一个静态成员变量 current_state_ptr，它存储当前状态机的指针。
    在状态机运行时，current_state_ptr 会指向当前状态（状态机的当前状态）。*/
    static state_ptr_t current_state_ptr;

    // public, leaving ability to access state instance (e.g. on reset)
    /*state() 是一个模板函数，用来返回某个状态 S 的引用。
    S 是状态类型，函数内部会检查模板参数 S 和 F 是否匹配，确保访问的状态属于同一个状态机（通过 is_same_fsm<F, S> 判断）。
    static_assert 确保模板参数的类型是匹配的，否则会抛出编译时错误。*/
    template<typename S>
    static constexpr S & state(void) {
      static_assert(is_same_fsm<F, S>::value, "accessing state of different state machine");
      return _state_instance<S>::value;
    }

    //判断当前 current_state_ptr 是否指向状态 S 的实例，返回一个布尔值。
    template<typename S>
    static constexpr bool is_in_state(void) {
      static_assert(is_same_fsm<F, S>::value, "accessing state of different state machine");
      return current_state_ptr == &_state_instance<S>::value;
    }

  /// state machine functions
  public:

    // explicitely specialized in FSM_INITIAL_STATE macro
    static void set_initial_state();

    static void reset() { };

    static void enter() {
      current_state_ptr->entry();
    }

    static void start() {
      set_initial_state();
      enter();
    }

    // dispatch() 用来派发事件 event 给当前状态处理, react() 函数来自于模板类型输入参数 F ，
    // F中需要有 react() 函数的定义， 否则在调用函数时，编译器会报错。 
    template<typename E>
    static void dispatch(E const & event) {
      current_state_ptr->react(event);
    }


  /// state transition functions
  // 状态转换函数，用于在状态机中进行状态转换
  // 提供了三种重载形式，支持不同的转换需求
  protected:

    template<typename S>
    void transit(void) {
      static_assert(is_same_fsm<F, S>::value, "transit to different state machine");
      current_state_ptr->exit();
      current_state_ptr = &_state_instance<S>::value;
      current_state_ptr->entry();
    }


    // 在状态转换过程中执行一个动作函数 action_function
    template<typename S, typename ActionFunction>
    void transit(ActionFunction action_function) {
      static_assert(is_same_fsm<F, S>::value, "transit to different state machine");
      current_state_ptr->exit();
      // NOTE: do not send events in action_function definisions.
      action_function();
      current_state_ptr = &_state_instance<S>::value;
      current_state_ptr->entry();
    }

    // 转换状态前先检查 条件函数 condition_function()，只有当条件为真时才会执行状态转换。
    template<typename S, typename ActionFunction, typename ConditionFunction>
    void transit(ActionFunction action_function, ConditionFunction condition_function) {
      if(condition_function()) {
        transit<S>(action_function);
      }
    }
  };


  template<typename F>
  typename Fsm<F>::state_ptr_t Fsm<F>::current_state_ptr;

  // --------------------------------------------------------------------------
  // 模板结构体 FsmList，接受 变长模板参数 FF，表示一个状态机类型的列表
  // FF 可以是一个或多个状态机类，例如 Fsm1, Fsm2, Fsm3 等。
  template<typename... FF>
  struct FsmList;

  // 空特化（递归结束的条件）
  // 模板结构体 FsmList 的特例化，用于处理空参数列表的情况。
  // 这个特例化定义了空参数列表时的状态机列表行为。
  // 它提供了空的 set_initial_state()、reset()、enter() 和 dispatch() 函数。
  template<> struct FsmList<> {
    static void set_initial_state() { }
    static void reset() { }
    static void enter() { }
    template<typename E>
    static void dispatch(E const &) { }
  };

  // FsmList 用于管理多个状态机实例。通过递归特化，FsmList 能够依次调用每个状态机的函数，从而实现对多个状态机的统一控制。
  // 递归特化：当 FsmList 中有多个状态机时，FsmList<F, FF...> 会处理第一个状态机 F，然后递归处理剩下的状态机（FF...）。
  // 模板结构体 FsmList 的通用定义，用于处理非空参数列表的情况。
  // 它接受一个状态机类型 F 和一个可变参数列表 FF，表示其他状态机类型。
  // 该结构体提供了对状态机列表的操作，包括设置初始状态、重置状态机、进入状态机和分发事件。
  template<typename F, typename... FF>
  struct FsmList<F, FF...>
  {
    using fsmtype = Fsm<F>;

    static void set_initial_state() {
      fsmtype::set_initial_state();
      FsmList<FF...>::set_initial_state();
    }

    static void reset() {
      F::reset();
      FsmList<FF...>::reset();
    }

    static void enter() {
      fsmtype::enter();
      FsmList<FF...>::enter();
    }

    static void start() {
      set_initial_state();
      enter();
    }

    template<typename E>
    static void dispatch(E const & event) {
      fsmtype::template dispatch<E>(event);
      FsmList<FF...>::template dispatch<E>(event);
    }
  };

  // --------------------------------------------------------------------------
  // StateList 是用于管理一组 状态实例 的结构体，它与 FsmList 类似，也是通过递归模板来实现对多个状态的管理。

  // 空特化（递归结束的条件）
  template<typename... SS> struct StateList;
  template<> struct StateList<> {
    static void reset() { }
  };

  // 递归特化：处理多个状态的情况
  template<typename S, typename... SS>
  struct StateList<S, SS...>
  {
    static void reset() {
      _state_instance<S>::value = S();  // 重置状态 S 的实例
      StateList<SS...>::reset();
    }
  };

  // --------------------------------------------------------------------------

  // MooreMachine 和 MealyMachine 是两种不同类型的有限状态机（FSM）的基类。
  // MooreMachine 基于 Moore 模型，状态的输出仅依赖于当前状态，不依赖于输入事件；
  // 而 MealyMachine 基于 Mealy 模型，状态的输出依赖于当前状态和输入事件。
  template<typename F>
  struct MooreMachine : tinyfsm::Fsm<F>
  {
    virtual void entry(void) { };  /* entry actions in some states */
    void exit(void) { };           /* no exit actions */
  };

  template<typename F>
  struct MealyMachine : tinyfsm::Fsm<F>
  {
    // input actions are modeled in react():
    // - conditional dependent of event type or payload
    // - transit<>(ActionFunction)
    void entry(void) { };  /* no entry actions */
    void exit(void) { };   /* no exit actions */
  };

} /* namespace tinyfsm */


// --------------------------------------------------------------------------
// FSM_INITIAL_STATE 宏特例化Fsm类中的set_initial_state函数，特例化后实现将current_state_ptr指向指定状态_STATE的实例。
// FSM_INITIAL_STATE 宏通过 模板特化 为指定的状态机类（_FSM）设置一个初始状态（_STATE）
#define FSM_INITIAL_STATE(_FSM, _STATE)                               \
namespace tinyfsm {                                                   \
  template<> void Fsm< _FSM >::set_initial_state(void) {              \
    current_state_ptr = &_state_instance< _STATE >::value;            \
  }                                                                   \
}

#endif /* TINYFSM_HPP_INCLUDED */
