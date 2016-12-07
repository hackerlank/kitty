local key     = KEYS[1]
local ttl     = KEYS[2]
print("key is"..key)
print("ttl is"..ttl)
local lockSet = redis.call('setnx', key, 'ok')
if lockSet == 1 then
    redis.call('expire', key, ttl)
end
return lockSet

