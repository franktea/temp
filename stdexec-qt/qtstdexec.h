#ifndef QTHREADSENDER_H
#define QTHREADSENDER_H

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <exec/async_scope.hpp>
#include <stdexec/concepts.hpp>
#include <stdexec/execution.hpp>
#include <exec/materialize.hpp>
#include <exec/start_now.hpp>
#include <exception>
#include <tuple>
#include <type_traits>

namespace QtStdExec {

// 前向声明：QThreadOperationState 模板类
template <class Recv>
class QThreadOperationState;

class QThreadScheduler
{
public:
    explicit QThreadScheduler(QThread* thread) : m_thread(thread) {}
    
    QThread* thread() {return m_thread;}
    
    struct default_env {
        QThread* thread;
        
        template <typename CPO>
        QThreadScheduler query(stdexec::get_completion_scheduler_t<CPO>) const noexcept {
            return QThreadScheduler(thread);
        }
    };
    
    // 这个sender用来执行普通的C++函数，后面还有一个QObjectSender专门用来执行Qt slot相关
    class QThreadSender
    {
    public:
        using sender_concept = stdexec::sender_t;
        
        using completion_signatures = stdexec::completion_signatures<
            stdexec::set_value_t(),
            stdexec::set_error_t(std::exception_ptr)>;

        explicit QThreadSender(QThread* thread) : m_thread(thread) {}
        QThread* thread() {return m_thread;}

        default_env get_env() const noexcept {
            return { m_thread };
        }

        template <class Recv>
        QThreadOperationState<Recv> connect(Recv&& receiver)
        {
            return QThreadOperationState<Recv>(std::move(receiver), thread());
        }
    private:
        QThread* m_thread;
    };

    QThreadSender schedule() const noexcept {
        return QThreadSender(m_thread);
    }
    
    // 相等比较操作符
    friend bool operator==(const QThreadScheduler& a, const QThreadScheduler& b) noexcept
    {return a.m_thread == b.m_thread;}
private:
    QThread* m_thread = nullptr;
};

inline QThreadScheduler qThreadAsScheduler(QThread* thread)
{
    return QThreadScheduler(thread);
}

inline QThreadScheduler qThreadAsScheduler(QThread& thread)
{
    return QThreadScheduler(&thread);
}

template <class Recv>
class QThreadOperationState
{
public:
    using operation_state_concept = stdexec::operation_state_t;
    
    QThreadOperationState(Recv&& receiver, QThread* thread) :
        m_receiver(std::move(receiver)), 
        m_thread(thread) {}
    
    void start() noexcept {
        QMetaObject::invokeMethod(
            m_thread->eventDispatcher(),
            [this]() {
                // 在目标线程中设置值，通知接收器操作完成
                stdexec::set_value(std::move(m_receiver));
            },
            Qt::QueuedConnection);  // 使用队列连接确保线程安全
    }
private:
    Q_DISABLE_COPY_MOVE(QThreadOperationState)  // 禁止拷贝和移动
    Recv m_receiver;    // 接收器对象
    QThread* m_thread;  // 目标线程
};

template <class Recv, class QObj, class Ret, class... Args>
class QObjectOperationState;

template <class QObj, class Ret, class... Args>
class QObjectSender
{
    struct default_env {
        QThread* thread;
        
        template <typename CPO>
        QThreadScheduler query(stdexec::get_completion_scheduler_t<CPO>) const noexcept {
            return QThreadScheduler(thread);
        }
    };
    
    default_env get_env() const noexcept {
        return { m_obj->thread() };
    }
public:
    using sender_concept = stdexec::sender_t;
    
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(Args...),
        stdexec::set_error_t(std::exception_ptr),
        stdexec::set_stopped_t()>;

    using m_ptr_type = Ret (QObj::*)(Args...);  // 成员函数指针类型
    
    // 构造函数：接收 Qt 对象和成员函数指针
    QObjectSender(QObj* obj, m_ptr_type ptr) : m_obj(obj), m_ptr(ptr) {}
    
    QObj* object() {return m_obj;}
    
    m_ptr_type member_ptr() {return m_ptr;}
    
    template <class Recv>
    QObjectOperationState<Recv, QObj, Ret, Args...>
    connect(Recv&& receiver)
    {
        return QObjectOperationState<Recv, QObj, Ret, Args...>(std::move(receiver), m_obj, m_ptr);
    }
private:
    QObj* m_obj;        // Qt 对象指针
    m_ptr_type m_ptr;   // 成员函数指针
};

template <class Recv, class QObj, class Ret, class... Args>
class QObjectOperationState
{
public:
    using operation_state_concept = stdexec::operation_state_t;
    using m_ptr_type = Ret (QObj::*)(Args...);

    QObjectOperationState(Recv&& receiver, QObj* obj, m_ptr_type ptr)
        : m_receiver(std::move(receiver)), m_obj(obj), m_ptr(ptr) {}
private:
    struct stop_callback_t {
        QObjectOperationState* self;

        void operator()() const noexcept {
            self->m_stop_callback.reset();
            QObject::disconnect(self->m_connection);
            if (!self->m_completed.test_and_set(std::memory_order_acq_rel)) {
                QMetaObject::invokeMethod(
                    self->m_obj->thread()->eventDispatcher(),
                    [this]() {
                        stdexec::set_stopped(std::move(self->m_receiver));
                    },
                    Qt::QueuedConnection);
            }
        }
    };
private:
    using stop_token_type = stdexec::stop_token_of_t<stdexec::env_of_t<Recv>>;
    using stop_callback_type = typename stop_token_type::template callback_type<stop_callback_t>;

public:
    void start() noexcept
    {
        // 设置停止回调，用于处理操作取消的情况
        // 当接收器的停止令牌被请求时，stop_callback_t 会被调用
        m_stop_callback.emplace(stdexec::get_stop_token(stdexec::get_env(m_receiver)), stop_callback_t{this});
        
        // 连接Qt对象的成员函数信号到lambda槽函数
        // 使用SingleShotConnection确保连接只触发一次
        m_connection = QObject::connect(m_obj, m_ptr, m_obj,
            [this](Args... args) {
                // 信号触发后的处理逻辑：
                // 1. 断开连接，避免重复触发
                QObject::disconnect(m_connection);
                // 2. 重置停止回调，因为操作即将完成
                m_stop_callback.reset();
                // 3. 使用原子标志确保操作只完成一次（防止竞态条件）
                if (!m_completed.test_and_set(std::memory_order_acq_rel)) {
                    // 4. 通过事件队列在目标线程中设置值到接收器
                    //    使用QueuedConnection确保线程安全
                    QMetaObject::invokeMethod(m_obj,
                        [this, &args...] {
                            // 将参数转发给接收器，标记操作成功完成
                            stdexec::set_value(std::move(m_receiver), std::forward<Args>(args)...);
                        },
                        Qt::QueuedConnection);
                }
            }, Qt::SingleShotConnection);
    }
    ~QObjectOperationState() {}
private:
    Recv m_receiver;
    QObj* m_obj;
    m_ptr_type m_ptr;
    QMetaObject::Connection m_connection;
    std::atomic_flag m_completed{false};
    std::optional<stop_callback_type> m_stop_callback;
};

template <class QObj, class Ret, class... Args>
inline QObjectSender<QObj, Ret, Args...> qObjectAsSender(QObj* obj, Ret (QObj::*ptr)(Args...))
{
    return QObjectSender<QObj, Ret, Args...>(obj, ptr);
}

template <class QObj, class Ret, class... Args>
inline auto qObjectAsTupleSender(QObj* obj, Ret (QObj::*ptr)(Args...))
{
    return QObjectSender<QObj, Ret, Args...>(obj, ptr)
            | stdexec::then([](Args... args){return std::tuple<std::remove_reference_t<Args>...>(std::move(args)...);});
}

struct QEventLoopWaitReceiver {
    using receiver_concept = stdexec::receiver_t;
    void set_value(auto&&...) noexcept {
    }
    void set_error(auto&&) noexcept {
    }
    void set_stopped() noexcept {
    }
};

template <class Sender>
auto qEventLoopWait(Sender&& sender)
{
    QEventLoop nested_loop;
    QTimer loop_end_timer{&nested_loop};
    loop_end_timer.setSingleShot(true);
    QObject::connect(&loop_end_timer, &QTimer::timeout,
                     [&] {nested_loop.quit();});
    auto wrapped_sender = std::forward<Sender>(sender) | exec::materialize();
    using result = stdexec::value_types_of_t<decltype(wrapped_sender)>;
    std::optional<result> res;
    auto result_sender = std::move(wrapped_sender) | stdexec::then([&res](auto tag, auto&&... args) {
        res.emplace(result(std::tuple(tag, std::forward<decltype(args)>(args)...)));
    }) | stdexec::continues_on(qThreadAsScheduler(QCoreApplication::instance()->thread()))
       | stdexec::then([&loop_end_timer](auto&&...) {loop_end_timer.start();});
    auto opstate = stdexec::connect(std::move(result_sender), QEventLoopWaitReceiver());
    stdexec::start(opstate);
    nested_loop.exec(); // 这里会阻塞等待，所以起到了同步调用的效果
    return res;
}

class QAsyncScopeGuard
{
private:
    exec::async_scope& m_scope;
public:
    QAsyncScopeGuard(exec::async_scope& scope) : m_scope(scope) {}
    ~QAsyncScopeGuard()
    {
        auto cleanupSender = m_scope.on_empty();
        m_scope.request_stop();
        qEventLoopWait(cleanupSender);
    }
};

}

#endif // QTHREADSENDER_H
