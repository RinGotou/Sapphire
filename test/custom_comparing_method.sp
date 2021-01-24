struct base
  attribute value # Bug? : null doesn't have a initializer
  
  fn initializer(value)
    me.value = value
  end
  
  fn compare(rhs)
    if typeid(rhs) != 'base'
      return false
    end
    
    return me.value == rhs.value
  end
end

obj0 = base(1)
obj1 = base(1)
obj2 = base(2)

if obj0.value == 1
  println('pass 0')
end

println(obj0 == obj1)
println(obj0 == obj2)
