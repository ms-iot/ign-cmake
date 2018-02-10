/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef IGNITION_UTILITIES_IMPLPTR_HH_
#define IGNITION_UTILITIES_IMPLPTR_HH_

#include <memory>

#include <ignition/utilities/detail/DefaultOps.hh>

namespace ignition
{
  namespace utilities
  {
    //////////////////////////////////////////////////
    /// \brief The ImplPtr class provides a convenient away to achieve the
    /// <a href="http://en.cppreference.com/w/cpp/language/rule_of_three">
    /// Rule of Zero</a> while keeping all the benefits of PIMPL. This saves us
    /// from writing an enormous amount of boilerplate code for each class.
    ///
    /// To follow PIMPL design principles, create an object of this type as the
    /// one and only member variable of your class, e.g.:
    ///
    /// \code
    /// class MyClass
    /// {
    ///   public: /* ... public member functions ... */
    ///
    ///   private: class Implementation;
    ///   private: ImplPtr<Implementation> dataPtr;
    /// };
    /// \endcode
    ///
    /// When constructing the \code{dataPtr} object, pass it
    /// \code{MakeImpl<Implementation>(/* ... args ... */)} in the
    /// initialization list of your class. \sa MakeImpl<T>()
    ///
    /// This class was inspired by the following blog post:
    /// http://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html
    template <class T,
              class Deleter = void (*)(T*),
              class Operations = detail::CopyMoveDeleteOperations<T> >
    class ImplPtr
    {
      /// \brief Constructor
      /// \tparam U A type that is compatible with T, i.e. either T or a class
      /// that is derived from T.
      /// \tparam D The deleter type
      /// \tparam Ops The copy operation container type
      /// \param[in] _ptr The raw pointer to the implementation
      /// \param[in] _deleter The deleter object
      /// \param[in] _ops The copy operation object
      public: template <class U, class D, class Ops>
      ImplPtr(U *_ptr, D &&_deleter, Ops &&_ops);

      /// \brief Copy constructor
      /// \param[in] _other Another ImplPtr of the same type
      public: ImplPtr(const ImplPtr &_other);

      /// \brief Copy assignment operator
      /// \param[in] _other Another ImplPtr of the same type
      /// \return A reference to this ImplPtr
      public: ImplPtr &operator=(const ImplPtr &_other);

      // We explicitly declare the move constructor to make it clear that it is
      // available.
      public: ImplPtr(ImplPtr &&) = default;

      // We explicitly declare the move assignment operator to make it clear
      // that it is available.
      public: ImplPtr &operator=(ImplPtr &&) = default;

      /// \brief Destructor
      public: ~ImplPtr() = default;

      /// \brief Non-const dereference operator. This const-unqualified operator
      /// ensures that logical const-correctness is followed by the consumer
      /// class.
      /// \return A mutable reference to the contained object.
      public: T &operator*();

      /// \brief Const dereference operator. This const-qualified operator
      /// ensures that logical const-correctness is followed by the consumer
      /// class.
      /// \return A const-reference to the contained object.
      public: const T &operator*() const;

      /// \brief Non-const member access operator. This const-unqualified
      /// operator ensures that logical const-correctness is followed by the
      /// consumer class.
      /// \return Mutable access to the contained object's members.
      public: T *operator->();

      /// \brief Const member access operator. This const-qualified operator
      /// ensures that logical const-correctness is followed by the consumer
      /// class.
      /// \return Immutable access to the contained object's members.
      public: const T *operator->() const;

      /// \internal \brief Create a clone of this ImplPtr's contents. This is
      /// for internal use only. The copy constructor and copy assignment
      /// operators should suffice for consumers.
      ///
      /// This function is needed internally for consumers' default copy
      /// constructors to compile.
      ///
      /// \return An ImplPtr that has been copied from the current one.
      private: ImplPtr Clone() const;

      /// \brief Pointer to the contained object
      private: std::unique_ptr<T, Deleter> ptr;

      /// \brief Structure to hold the copy operators
      private: Operations ops;
    };

    //////////////////////////////////////////////////
    /// \brief Pass this to the constructor of an ImplPtr object to easily
    /// initialize it. All the arguments passed into this function will be
    /// perfectly forwarded to the implementation class that gets created.
    ///
    /// E.g.:
    ///
    /// \code
    /// MyClass::MyClass(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    ///   : dataPtr(utilities::MakeImpl<Implementation>(arg1, arg2, arg3))
    /// {
    ///   // Do nothing
    /// }
    /// \endcode
    ///
    /// \tparam T The typename of the implementation class. This must be set
    /// explicitly.
    /// \tparam Args The argument types. These will be inferred automatically.
    /// \param[in] _args The arguments to be forwarded to the implementation
    /// class.
    /// \return A new ImplPtr<T>. Passing this along to a class's ImplPtr
    /// object's constructor will efficiently move this newly created object
    /// into it.
    template <class T, typename... Args>
    ImplPtr<T> MakeImpl(Args &&..._args);

    //////////////////////////////////////////////////
    /// \brief This is an alternative to ImplPtr<T> which serves the same
    /// purpose, except it only provide move semantics (i.e. it does not allow
    /// copying). This should be used in cases where it is not safe (or not
    /// possible) to copy the underlying state of an implementation class.
    ///
    /// Note that when creating an implementation class that is unsafe to copy,
    /// you should explicitly delete its copy constructor (unless one of its
    /// members has an explicitly deleted copy constructor). Doing so will force
    /// you to use UniqueImplPtr instead of ImplPtr, and it will signal to
    /// future developers or maintainers that the implementation class is not
    /// meant to be copiable.
    ///
    /// Use MakeUniqueImpl<T>() to construct UniqueImplPtr objects.
    template <class T, class Deleter = void (*)(T*)>
    using UniqueImplPtr = std::unique_ptr<T, Deleter>;

    //////////////////////////////////////////////////
    /// \brief Pass this to the constructor of a UniqueImplPtr object to easily
    /// initialize it. All the arguments passed into this function will be
    /// perfectly forwarded to the implementation class that gets created.
    ///
    /// E.g.:
    ///
    /// \code
    /// MyClass::MyClass(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    ///   : dataPtr(utilities::MakeUniqueImpl<Implementation>(arg1, arg2, arg3))
    /// {
    ///   // Do nothing
    /// }
    /// \endcode
    ///
    /// \tparam T The typename of the implementation class. This must be set
    /// explicitly.
    /// \tparam Args The argument types. These will be inferred automatically.
    /// \param[in] _args The arguments to be forwarded to the implementation
    /// class.
    /// \return A new UniqueImplPtr<T>. Passing this along to a class's
    /// UniqueImplPtr object's constructor will efficiently move this newly
    /// created object into it.
    template <class T, typename... Args>
    UniqueImplPtr<T> MakeUniqueImpl(Args &&...args);
  }
}

#include <ignition/utilities/detail/ImplPtr.hh>

#endif
