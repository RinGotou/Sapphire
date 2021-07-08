fn SampleFunc(value) -> int 
  attribute result
  if   value == 1
	result = 'hello'
	println('first condition')
  elif value == 2
	result = 2
	println('second condition')
  end
  return result
end

fn SampleWithoutConstraint()
  return 'Sample'
end

println('1:' + SampleWithoutConstraint())
println('2:' + SampleFunc(2))
println('3:' + SampleFunc(1)) #boom!
#TODO: fix missing error message for caller