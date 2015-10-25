function sq(a:uint8):uint16
	return a * a
end

function main():void
--	print("Hello", "world")
--	print(1*4+3/2+3*2*1+1) 
	printInt16(sq(5))
  printInt32(sq(6))
  return
end