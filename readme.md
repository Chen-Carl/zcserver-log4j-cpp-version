## 定义日志器

``` cpp
zcserver::Logger g_logger = zcserver::LoggerMgr::GetInstance()->getLogger(name);

ZCSERVER_LOG_INFO(g_logger) << "log";
```

创建一个logger
``` cpp
zcserver::Logger g_logger = ZCSERVER_LOG_NAME("system");
```
当g_logger的appender为空时，使用root的配置写日志。