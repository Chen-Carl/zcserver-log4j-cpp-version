#include <iostream>
#include <thread>
#include "../src/log.h"

int main() 
{
    // 测试流式输出
    std::cout << "stream log" << std::endl;
    // 构造一个默认的日志器
    std::shared_ptr<zcserver::Logger> logger(new zcserver::Logger);
    // 给日志器添加标准输入
    logger->addAppender(std::shared_ptr<zcserver::LogAppender>(new zcserver::StdoutLogAppender));
    // 构造一个日志事件
    std::shared_ptr<zcserver::LogEvent> event(new zcserver::LogEvent(logger, zcserver::LogLevel::DEBUG, __FILE__, __LINE__, 0, zcserver::GetThreadId(), zcserver::GetFiberId(), time(0)));

    ZCSERVER_LOG_INFO(logger) << "test macro info";
    ZCSERVER_LOG_ERROR(logger) << "test macro error";

    // 结束流式输出的测试
    std::cout << "end stream log" << std::endl;

    // 测试格式化输出
    std::shared_ptr<zcserver::FileLogAppender> file_appender(new zcserver::FileLogAppender("./log.txt"));
    // 构造一个新格式
    std::shared_ptr<zcserver::LogFormatter> fmt(new zcserver::LogFormatter("%d%T%p%T%m%n"));

    // 更改文件的日志级别，希望输出严重的日志内容
    file_appender->setLevel(zcserver::LogLevel::ERROR);
    // 为文件输出设置输出格式
    file_appender->setFormatter(fmt);
    // 给日志器添加文件输出
    logger->addAppender(file_appender);
    ZCSERVER_LOG_FMT_INFO(logger, "test macro fmt error %s%s", "abc", "def");
    ZCSERVER_LOG_FMT_ERROR(logger, "test macro fmt error %s%s", "abc", "def");
    ZCSERVER_LOG_FMT_ERROR(logger, "test macro fmt error %s%s", "abc", "def");

    auto l = zcserver::LoggerMgr::GetInstance()->getLogger("xx");
    ZCSERVER_LOG_INFO(l) << "xxx";
    return 0;
}
