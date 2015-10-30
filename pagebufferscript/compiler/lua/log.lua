local log = {}

log.enabled = {
  info = true;
  warn = true;
}

setmetatable(log,{__index = function(t, k)
      if t.enabled[k] == nil then return end
      if t.enabled[k] then
        return function (self, ...)
          print(k:upper()..":",...)
        end
      else
        return function() end
      end
    end})

return log