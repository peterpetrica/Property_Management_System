#ifndef TOKENS_H
#define TOKENS_H

#include "db/database.h"
#include <stddef.h>
#include <stdbool.h>

// 生成令牌
char *generate_token(Database *db, const char *user_id, int user_type);

// 验证令牌并返回用户ID
bool get_user_id_from_token(Database *db, const char *token, char *user_id, size_t user_id_size);

// 使令牌失效
bool invalidate_token(Database *db, const char *token);

// 清理过期令牌
int cleanup_expired_tokens(Database *db);

#endif /* TOKENS_H */
