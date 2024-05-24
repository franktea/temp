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

using namespace stdexec::tags;

template <class Recv>
class QThreadOperation;


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
        auto connect(Recv receiver) {
            return QThreadOperation<Recv>(std::move(receiver), thread_);
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
class QThreadOperation {
public:
    QThreadOperation(Recv&& receiver, QThread* thread):
        receiver_(std::move(receiver)),
        thread_(thread) {}

    void start() noexcept {
        QMetaObject::invokeMethod(thread_->eventDispatcher(),
            [this]() { stdexec::set_value(std::move(receiver_)); },
            Qt::QueuedConnection);
    }
private:
    Q_DISABLE_COPY_MOVE(QThreadOperation)
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

    default_env get_env() const noexcept {
        return { obj_->thread() };
    }

public:
    //using is_sender = void;
    using sender_concept = stdexec::sender_t;
    using completion_signatures = stdexec::completion_signatures<
                                  stdexec::set_value_t(Args...),
        stdexec::set_error_t(std::exception_ptr)>;

    using m_ptr_type = Ret (QObj::*)(Args...);

    QObjectSender(QObj* obj, m_ptr_type ptr) : obj_(obj), m_ptr_(ptr) {}

    //template<class Recv>
    //friend inline QObjectOperationState<Recv, QObj, Ret, Args...>
    //tag_invoke(stdexec::tag_t<stdexec::connect>, QObjectSender sender, Recv&& receiver) {
    //    return QObjectOperationState<Recv, QObj, Ret, Args...>(std::move(receiver), sender.obj_, sender.m_ptr_);
    //}

    STDEXEC_MEMFN_DECL(auto connect)(this auto sender, stdexec::receiver auto&& receiver) {
        return QObjectOperationState<decltype(receiver), QObj, Ret, Args...>(std::move(receiver), sender.obj_, sender.m_ptr_);
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
        receiver_(std::move(receiver)),
        obj_(obj),
        m_ptr_(ptr) {}

    //friend void tag_invoke(stdexec::tag_t<stdexec::start>, QObjectOperationState& state) noexcept {
    void start() noexcept {
        connection_ = QObject::connect(obj_, m_ptr_,
            [this](Args... args) {
                stdexec::set_value(std::move(receiver_), std::forward<Args>(args)...);
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
