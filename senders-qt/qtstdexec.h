#ifndef QTSTDEXEC_H
#define QTSTDEXEC_H

#include <QAbstractEventDispatcher>
#include <QMetaObject>
#include <QObject>
#include <QThread>
#include "stdexec/concepts.hpp"
#include "stdexec/execution.hpp"
#include <exception>
#include <tuple>
#include <type_traits>

namespace QtSteExec {

template <class Recv>
class QThreadOperationState;


class QThreadScheduler {
public:
    explicit QThreadScheduler(QThread* thread) : thread_(thread) {}
    QThread* Thread() { return thread_; }

    struct default_env {
        QThread* thread;
        template<typename CPO>
            friend QThreadScheduler tag_invoke(stdexec::get_completion_scheduler_t<CPO>,
                                           default_env env) noexcept {
            return QThreadScheduler(env.thread);
        }
    };

    class QThreadSender {
    public:
        using is_sender = void;
        using completion_signatures = stdexec::completion_signatures<
                                      stdexec::set_value_t(),
            stdexec::set_error_t(std::exception_ptr)>;

        explicit QThreadSender(QThread* thread) : thread_(thread) {}

        friend default_env tag_invoke(stdexec::get_env_t, const QThreadSender& snd) noexcept {
            return { snd.thread_ };
        }

        template<class Recv>
        friend inline QThreadOperationState<Recv> tag_invoke(stdexec::connect_t, QThreadSender sender, Recv&& receiver) {
            return QThreadOperationState<Recv>(std::move(receiver), sender.thread_);
        }

        friend QThreadSender tag_invoke(stdexec::schedule_t, QThreadScheduler sched) {
            return QThreadSender(sched.Thread());
        }
    private:
        QThread* thread_ = nullptr;
    };

    friend auto operator<=>(const QThreadScheduler&, const QThreadScheduler&) = default;
private:
    QThread* thread_ = nullptr;
};

inline QThreadScheduler QThreadAsScheduler(QThread* thread) {
    return QThreadScheduler(thread);
}

inline QThreadScheduler QThreadAdScheduler(QThread& thread) {
    return QThreadScheduler(&thread);
}

template<class Recv>
class QThreadOperationState {
public:
    QThreadOperationState(Recv&& receiver, QThread* thread):
        receiver_(std::move(receiver)),
        thread_(thread) {}

    void start() noexcept {
        QMetaObject::invokeMethod(thread_->eventDispatcher(),
            [this]() { stdexec::set_value(std::move(receiver_)); },
            Qt::QueuedConnection);
    }

    friend void tag_invoke(stdexec::tag_t<stdexec::start>, QThreadOperationState& state) noexcept {
        state.start();
    }
private:
    Q_DISABLE_COPY_MOVE(QThreadOperationState)
    Recv receiver_;
    QThread* thread_;
};

template<class Recv, class QObj, class Ret, class... Args>
class QObjectOperationState;

template<class QObj, class Ret, class... Args>
class QObjectSender {
    struct default_env {
        QThread* thread;
        template<typename CPO>
        friend QThreadScheduler tag_invoke(stdexec::get_completion_scheduler_t<CPO>,
                                           default_env env) noexcept {
            return QThreadScheduler(env.thread);
        }
    };

    friend default_env tag_invoke(stdexec::get_env_t, const QObjectSender& snd) noexcept {
        return { snd.obj_->thread() };
    }

public:
    using is_sender = void;
    using m_ptr_type = Ret (QObj::*)(Args...);
    using completion_signatures = stdexec::completion_signatures<
                                  stdexec::set_value_t(Args...),
        stdexec::set_error_t(std::exception_ptr)>;

    QObjectSender(QObj* obj, m_ptr_type ptr) : obj_(obj), m_ptr_(ptr) {}
    QObj* Object() { return obj_; }
    m_ptr_type MemberPtr() { return m_ptr_; }

    template<class Recv>
    friend inline QObjectOperationState<Recv, QObj, Ret, Args...>
    tag_invoke(stdexec::tag_t<stdexec::connect>, QObjectSender sender, Recv&& receiver) {
        return QObjectOperationState<Recv, QObj, Ret, Args...>(std::move(receiver), sender.Object(), sender.MemberPtr());
    }

private:
    QObj* obj_;
    m_ptr_type m_ptr_;

};

template<class Recv, class QObj, class Ret, class... Args>
class QObjectOperationState {
public:
    using m_ptr_type = Ret (QObj::*)(Args...);
    QObjectOperationState(Recv&& receiver, QObj* obj, m_ptr_type ptr) :
                                                                        receiver_(receiver),
                                                                        obj_(obj),
                                                                        m_ptr_(ptr) {}

    friend void tag_invoke(stdexec::tag_t<stdexec::start>, QObjectOperationState& state) noexcept {
        state.connection_ = QObject::connect(state.obj_, state.m_ptr_,
                                             [&state](Args... args) {
                                                 stdexec::set_value(std::move(state.receiver_), std::forward<Args>(args)...);
                                             });
    }

    ~QObjectOperationState() {
        QObject::disconnect(connection_);
    }
private:
    Recv receiver_;
    QObj* obj_;
    m_ptr_type m_ptr_;
    QMetaObject::Connection connection_;
};

template<class QObj, class Ret, class... Args>
inline QObjectSender<QObj, Ret, Args...> QObjectAsSender(QObj* obj, Ret (QObj::* ptr)(Args...)) {
    return QObjectSender<QObj, Ret, Args...>(obj, ptr);
}

template<class QObj, class Ret, class... Args>
inline auto QObjectAsTupleSender(QObj* obj, Ret (QObj::*ptr)(Args...)) {
    return QObjectSender<QObj, Ret, Args...>(obj, ptr)
           | stdexec::then([](Args... args){
               return std::tuple<std::remove_reference_t<Args>...>(std::move(args)...);
           });
}

} // namespace QtSteExec

#endif // QTSTDEXEC_H
