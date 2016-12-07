if redis.call("get", KEYS[1]) == 'ok'   then
    return redis.call("del", KEYS[1])
else
    return 0
end

