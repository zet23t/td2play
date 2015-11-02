local function test(file)
  print("Compiling "..file)
  local PBSCompiler = require "pbs.PBSCompiler"
  local prog = PBSCompiler:new()
  prog:addNativeFunction(0, "printInt16","void") "int16"
  prog:addNativeFunction(1, "printInt32","void") "int32"
  
  local code = prog:compile("test/"..file..".pbs")
  prog.instructionCode:printInstructions()
  
  local fp = io.open("test/"..file..".pbs","rb")
  local content = fp:read "*a"
  
  local expectedOutput = {}
  local expectedOutputPos = 1
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

local function runTests(files)
  for i,v in ipairs(files) do
    test(v)
  end
end

runTests {"funcdef", "locals"}

