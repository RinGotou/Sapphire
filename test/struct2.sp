slash = '----------------'

struct model
  id = '(none)'
  
  fn initializer(id)
    println('Init model object')
    me.id = id
    println('model end')
  end
end

struct phone
  model_ = null()
  
  fn initializer(id)
    println('Init phone object')
    me.model_ = model(id)
    println(me.model_.id)
    println('phone end')
  end
  
  fn print_model()
    println(me.model_.id)
  end
end

sample1 = phone('F5321')
println(sample1.model_.id)
println(slash)

for unit in methods(phone)
  println(unit)
end
println(slash)

for unit in phone.members()
  println(unit)
end
println(slash)


println(exist(phone, 'initializer'))
sample1.print_model()
