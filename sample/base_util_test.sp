struct base;end
struct inherited < base;end

fn test()
  attribute obj0
  println(obj0)
  print('Type something >')
  local obj1 = input()
  println(obj1)
  console('winver')
  println(timestring())
  println(is_base_of(base, inherited))
  println(core_version())
  println(core_codename())
end

test()