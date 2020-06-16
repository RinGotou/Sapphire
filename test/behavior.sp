using 'types'

struct base
  attribute value
  
  fn initializer(value)
    me.value = value
  end
  
  fn compare(rhs)
    if typeid(rhs) != 'base'
      return false
    end
    
    return me.value == rhs.value
  end
  
  fn print()
    println('Printing method test')
  end
end

obj0 = base(1)
obj1 = base(1)
obj2 = base(2)

if obj0.value == 1
  println('pass 0')
end

println('comparing test')

println(obj0 == obj1)
println(obj0 == obj2)

println(obj0)

i = 1
println('Plain Type: ' + is_plain_type(i))
println('Plain Type: ' + is_plain_type(obj0))
