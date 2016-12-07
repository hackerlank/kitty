if redis.call("get", KEYS[1]) == 'ok'   then
    return 1
else
    return 0
end

