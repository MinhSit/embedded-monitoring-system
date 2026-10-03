#ifndef LOG_H
#define LOG_H

typedef enum{
  LOG_LEVEL_INFO = 0U,
  LOG_LEVEL_WARN = 1U,
  LOG_LEVEL_ERROR = 2U
} log_level_t;

void log_write(log_level_t level, const char *msg);

void log_init(void);

#endif /* LOG_H */
