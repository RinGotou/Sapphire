struct base
  fn initializer(value)
    println('Super struct:' + value)
  end
end

struct sub < base
  fn initializer()
    super('from sub')
    println('Sub struct')
  end
end

sample = sub()