local function test(file)
  print("\nCompiling "..file)
  local PBSCompiler = require "pbs.PBSCompiler"
  
  local prog = PBSCompiler:new()
  prog:addNativeFunction(0, "printInt16","void") "int16"
  prog:addNativeFunction(1, "printInt32","void") "int32"
  
  local expectedOutputPos = 1
  local expectedOutput = {}
  local function compileAndRun()
    local code = prog:compile("test/"..file..".pbs")
    --prog.instructionCode:printInstructions()
    
    local fp = io.open("test/"..file..".pbs","rb")
    local content = fp:read "*a"
    
    local env = {}
    function env.expectedOutput(tab)
      expectedOutput = tab
    end
    
    local function checkExpected(v)
      local e = expectedOutput[expectedOutputPos]
      expectedOutputPos = expectedOutputPos + 1
      assert(e == v, "Expected Output: "..e..", got "..tostring(v))
    end
    
    for line in content:gmatch "%-%-#([^\n]+)" do
      local fn = assert(loadstring(line,"@test/"..file))
      setfenv(fn,env)
      fn()
    end
    
    local VM = require "pbsinterpreter"
    local vm = VM:new()
    vm:load(code)
    vm:addFunction(0,function(vm)
          checkExpected(vm:readInt16FromStack(0))
        end, 2, 0)
    vm:addFunction(1,function(vm) 
          checkExpected(vm:readInt32FromStack(0))
        end, 4, 0)
    vm:execute()
    assert(expectedOutputPos > #expectedOutput,
      "Missing expected output: ".. tostring(expectedOutput[expectedOutputPos]))
    print("  checks done: "..(expectedOutputPos-1))
  end
  xpcall(compileAndRun, function(err)
      print "Error during test:"
      print(debug.traceback(err))
      print("  checks succeeded: ".. expectedOutputPos - 1 .."/".. #expectedOutput)
      if prog.instructionCode and prog.instructionCode.code then
        print "Compiled Bytecode: "
        prog.instructionCode:printByteCode()
      end
      
    end)
end

local function runTests(files)
  for i,v in ipairs(files) do
    test(v)
  end
end

runTests {"funcdef", "locals"}

