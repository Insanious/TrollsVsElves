#pragma once

// yoinked from https://github.com/victorsummer/SimpleSignal

#include <functional>
#include <vector>

namespace simpleSignal {
template<int>
struct memberFunctionPlaceholder {};
}

namespace std {
    template<int N>
    struct is_placeholder<simpleSignal::memberFunctionPlaceholder<N>> : std::integral_constant<int, N + 1> {};
}

namespace simpleSignal {

template<int... Ns>
struct int_sequence {};

template<int N, int... Ns>
struct make_int_sequence : make_int_sequence<N - 1, N - 1, Ns...> {};

template<int... Ns>
struct make_int_sequence<0, Ns...> : int_sequence<Ns...> {};

template<class> class Signal;

template<class ResultType, class... Args>
class Signal<ResultType(Args...)> {
public:
    using CallbackType = std::function<ResultType(Args...)>;

    Signal() {}
    ~Signal() {}

    void connect(const Signal::CallbackType &func) {
        mCallbacks.emplace_back(std::move(func));
    }

    template<class T, class F>
    void connect(T *object, const F &memberFunc) {
        mCallbacks.emplace_back(constructMemberFunc(object, memberFunc, make_int_sequence<sizeof...(Args)>{}));
    }

    void disconnect() {
        mCallbacks.clear();
    }

    void emit(Args... args) {
        for (const auto &callback : mCallbacks) {
            callback(std::forward<Args>(args)...);
        }
    }

private:
    template<class T, class F, int... Ns>
    Signal::CallbackType constructMemberFunc(T *object, const F &memberFunc, int_sequence<Ns...>) const {
        return std::bind(memberFunc, object, memberFunctionPlaceholder<Ns>{}...);
    }

private:
    std::vector<Signal::CallbackType> mCallbacks;
};

}
