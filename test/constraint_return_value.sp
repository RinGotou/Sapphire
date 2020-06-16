fn SampleFunc(value) -> int 
  attribute result
  if   value == 1; result = 'hello'
  elif value == 2; result = 2
  end
  return result
end

fn SampleWithoutConstraint()
  println('Sample')
end

SampleWithoutConstraint()
SampleFunc(2)
SampleFunc(1)