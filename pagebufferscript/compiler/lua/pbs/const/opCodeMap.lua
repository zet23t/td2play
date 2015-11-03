local map = {}
local function op (name)
  map[name] = {}
  return function (description)
    map[name].description = description
    return {
      code = function(self, op)
        map[name].op = op
        return self
      end;
      arg = function(self, n)
        map[name].argBytes = n
        return self
      end;
    }
  end
end

op "push-8bit-literal" "Push 8bit instruction on top of stack" :code(0x01) :arg(1)
op "push-16bit-literal" "Push 16bit instruction on top of stack" :code(0x02) :arg(2)
op "push-32bit-literal" "Push 32bit instruction on top of stack" :code(0x03) :arg(4)
--op "push-string-literal" "<info>" :code(0x04) :arg()
--op "push-8bit-value" "<info>" :code(0x05) :arg()
--op "push-16bit-value" "<info>" :code(0x06) :arg()
op "push-stack-value-uint8" "<info>" :code(0x07) :arg()
op "push-stack-value-int8" "<info>" :code(0x0a) :arg()
op "op-*-uint8-uint8-uint16" "<info>" :code(0x08) :arg()
op "op-*-int8-int8-int8" "<info>" :code(0x0b) :arg()
op "return-uint16" "<info>" :code(0x09) :arg()
op "call" "<info>" :code(0x10) :arg()
op "callNative" "<info>" :code(0x11) :arg()
op "return-void" "<info>" :code(0x12) :arg()
op "move-function-stack-offset" "<info>" :code(0x13) :arg()
op "cast-uint16-int16" "<info>" :code(0x14) :arg()
op "cast-uint16-int32" "<info>" :code(0x15) :arg()
op "cast-int8-int16" "<info>" :code(0x16) :arg()

return map