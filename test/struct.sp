struct character
  attribute name, img
  
  fn initializer(name, img)
    println('initializer processing')
    me.name = name
    me.img = img
  end
  
  fn print_name()
    println(me.name)
  end
end

shiroha = character('shiroha', 'shiroha.png')
println(shiroha.name + ' ' + shiroha.img)
println(shiroha.name)
shiroha.print_name()


